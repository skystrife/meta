/**
 * @file disk_index.tcc
 * @author Sean Massung
 */

#include <cstdio>
#include <numeric>
#include <queue>
#include <iostream>
#include <utility>
#include <sys/stat.h>
#include "index/disk_index.h"
#include "index/chunk.h"
#include "parallel/thread_pool.h"
#include "util/common.h"
#include "util/optional.h"

namespace meta {
namespace index {

template <class Index, class... Args>
Index make_index(const std::string & config_file, Args &&... args)
{
    auto config = cpptoml::parse_file(config_file);

    // check if we have paths specified for either kind of index
    if (!(config.contains("forward-index")
          && config.contains("inverted-index"))) {
        throw typename Index::disk_index_exception{
            "forward-index or inverted-index missing from configuration file"
        };
    }

    Index idx{config, std::forward<Args>(args)...};

    // if index has already been made, load it
    if(mkdir(idx._index_name.c_str(), 0755) == -1)
        idx.load_index();
    else
        idx.create_index(config_file);
    return idx;
}

template <class Index, template <class, class> class Cache, class... Args>
cached_index<Index, Cache> make_index(const std::string & config_file,
                                      Args &&... args) {
    return make_index<cached_index<Index, Cache>>(config_file,
                                                  std::forward<Args>(args)...);
}

template <class DerivedIndex>
disk_index<DerivedIndex>::disk_index(const cpptoml::toml_group & config,
                                     const std::string & index_path):
    _tokenizer{ tokenizers::tokenizer::load_tokenizer(config) },
    _index_name{ index_path }
{ /* nothing */ }

template <class DerivedIndex>
std::string disk_index<DerivedIndex>::index_name() const
{
    return _index_name;
}

template <class DerivedIndex>
uint64_t disk_index<DerivedIndex>::unique_terms(doc_id d_id) const
{
    return _unique_terms->at(d_id);
}

template <class DerivedIndex>
uint64_t disk_index<DerivedIndex>::unique_terms() const
{
    return _tokenizer->num_terms();
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::create_index(const std::string & config_file)
{
    // save the config file so we can recreate the tokenizer
    std::ifstream source_config{config_file.c_str(), std::ios::binary};
    std::ofstream dest_config{_index_name + "/config.toml", std::ios::binary};
    dest_config << source_config.rdbuf();

    // load the documents from the corpus
    auto docs = corpus::corpus::load(config_file);

    uint64_t num_docs = docs->size();
    _doc_id_mapping.resize(num_docs);

    _doc_sizes = common::make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts", num_docs);
    _labels = common::make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels", num_docs);
    _unique_terms = common::make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms", num_docs);

    _term_bit_locations.reserve(num_docs * 3); // guess 3x

    // create postings file
    uint32_t num_chunks = tokenize_docs(docs.get());
    merge_chunks(num_chunks, _index_name + "/postings.index");
    compress(_index_name + "/postings.index");

    common::save_mapping(_doc_id_mapping, _index_name + "/docids.mapping");

    common::save_mapping(_term_bit_locations, _index_name + "/lexicon.index");
    common::save_mapping(_label_ids, _index_name + "/labelids.mapping");
    common::save_mapping(_compression_mapping,
            _index_name + "/keys.compressedmapping");
    _tokenizer->save_term_id_mapping(_index_name + "/termids.mapping");

    _postings = common::make_unique<io::mmap_file>(_index_name + "/postings.index");
}

template <class DerivedIndex>
uint32_t disk_index<DerivedIndex>::tokenize_docs(corpus::corpus * docs)
{
    std::mutex mutex;
    std::atomic<uint32_t> chunk_num{0};
    auto task = [&]() {
        typename DerivedIndex::chunk_handler handler{this, chunk_num};
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};
                if (!docs->has_next())
                    return; // destructor for handler will write
                            // any intermediate chunks
                doc = docs->next();
                std::string progress = " Documents: "
                    + common::add_commas(common::to_string(doc->id()))
                    + " Unique primary keys: "
                    + common::add_commas(
                        common::to_string(_tokenizer->num_terms()))
                    + " Tokenizing: ";
                common::show_progress(doc->id(), docs->size(), 50, progress);
            }

            _tokenizer->tokenize(*doc);

            // save metadata
            _doc_id_mapping[doc->id()] = doc->path();
            (*_doc_sizes)[doc->id()] = doc->length();
            (*_unique_terms)[doc->id()] = doc->frequencies().size();
            (*_labels)[doc->id()] = get_label_id(doc->label());

            // update chunk
            handler(*doc);
        }
    };

    parallel::thread_pool pool;
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < pool.thread_ids().size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto & fut : futures)
        fut.get();

    std::string progress = " Documents: "
        + common::add_commas(common::to_string(docs->size()))
        + " Unique primary keys: "
        + common::add_commas(common::to_string(_tokenizer->num_terms()))
        + " Tokenizing: ";
    common::end_progress(progress);

    return chunk_num;
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::calc_compression_mapping(
        const std::string & filename)
{
    std::ifstream in{filename};
    postings_data_type pdata{primary_key_type{0}};
    std::unordered_map<uint64_t, uint64_t> freqs;

    while(in >> pdata)
    {
        for(auto & c: pdata.counts())
        {
            ++freqs[c.first];
            ++freqs[*reinterpret_cast<const uint64_t*>(&c.second)];
        }
    }

    using pair_t = std::pair<uint64_t, uint64_t>;
    // TODO this requires 2x the memory of _compression_mapping which is already
    // huge!
    std::vector<pair_t> sorted{freqs.begin(), freqs.end()};
    std::sort(sorted.begin(), sorted.end(),
        [](const pair_t & a, const pair_t & b) {
            return a.second > b.second;
        }
    );

    _compression_mapping.clear();

    // have to know what the delimiter is, and can't use 0
    uint64_t delim = std::numeric_limits<uint64_t>::max();
    _compression_mapping.insert(delim, 1);

    // 2 is the first valid compressed char after the delimiter 1
    uint64_t counter = 2;
    for(auto & p: sorted)
        _compression_mapping.insert(p.first, counter++);
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::compress(
        const std::string & filename)
{
    std::cerr << "Calculating optimal compression mapping..." << std::endl;

    calc_compression_mapping(filename);
    std::string cfilename{filename + ".compressed"};

    std::cerr << "Creating compressed postings file..." << std::endl;

    // create scope so the writer closes and we can calculate the size of the
    // file as well as rename it
    {
        io::compressed_file_writer out{cfilename, _compression_mapping};

        postings_data_type pdata{primary_key_type{0}};
        std::ifstream in{filename};

        // note: we will be accessing pdata in sorted order
        while(in >> pdata)
        {
            _term_bit_locations.push_back(out.bit_location());
            pdata.write_compressed(out);
        }
    }

    std::cerr << "Created compressed postings file ("
              << common::bytes_to_units(common::file_size(cfilename))
              << ")" << std::endl;

    remove(filename.c_str());
    rename(cfilename.c_str(), filename.c_str());
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::load_index()
{
    std::cerr << "Loading index from disk ("
         << _index_name << ")..." << std::endl;

    auto config = cpptoml::parse_file(_index_name + "/config.toml");

    _doc_sizes = common::make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts");
    _labels = common::make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels");
    _unique_terms = common::make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms");

    common::load_mapping(_doc_id_mapping, _index_name + "/docids.mapping");
    common::load_mapping(_term_bit_locations, _index_name + "/lexicon.index");
    common::load_mapping(_label_ids, _index_name + "/labelids.mapping");
    common::load_mapping(_compression_mapping,
            _index_name + "/keys.compressedmapping");
    _tokenizer = tokenizers::tokenizer::load_tokenizer(config);
    _tokenizer->set_term_id_mapping(_index_name + "/termids.mapping");

    _postings = common::make_unique<io::mmap_file>(
        _index_name + "/postings.index"
    );
}

template <class DerivedIndex>
class_label disk_index<DerivedIndex>::label(doc_id d_id) const
{
    return class_label_from_id(_labels->at(d_id));
}

template <class DerivedIndex>
class_label disk_index<DerivedIndex>::class_label_from_id(label_id l_id) const
{
    return _label_ids.get_key(l_id);
}

template <class DerivedIndex>
label_id disk_index<DerivedIndex>::get_label_id(const class_label & lbl)
{
    std::mutex mutex;
    std::lock_guard<std::mutex> lock{mutex};
    if(!_label_ids.contains_key(lbl))
    {
        label_id next_id{static_cast<label_id>(_label_ids.size())};
        _label_ids.insert(lbl, next_id);
        return next_id;
    }
    else
        return _label_ids.get_value(lbl);
}

template <class DerivedIndex>
label_id disk_index<DerivedIndex>::label_id_from_doc(doc_id d_id) const
{
    return _labels->at(d_id);
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::write_chunk(
        uint32_t chunk_num,
        std::vector<postings_data_type> & pdata)
{
    std::sort(pdata.begin(), pdata.end());

    std::ofstream outfile{"chunk-" + common::to_string(chunk_num)};
    for(auto & p: pdata)
        outfile << p;
    outfile.close();

    pdata.clear();
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::merge_chunks(
        uint32_t num_chunks, const std::string & filename)
{
    using chunk_t = chunk<primary_key_type, secondary_key_type>;

    // create priority queue of all chunks based on size
    std::priority_queue<chunk_t> chunks;
    for(uint32_t i = 0; i < num_chunks; ++i)
    {
        std::string filename = "chunk-" + common::to_string(i);
        chunks.push(chunk_t{filename});
    }

    // merge the smallest two chunks together until there is only one left
    // done in parallel

    // this represents the number of merge steps needed---it is equivalent
    // to the number of internal nodes in a binary tree with n leaf nodes
    size_t remaining = chunks.size() - 1;
    std::mutex mutex;
    parallel::thread_pool pool;
    auto thread_ids = pool.thread_ids();
    std::vector<std::future<void>> futures;
    auto task = [&]() {
        while (true) {
            util::optional<chunk_t> first;
            util::optional<chunk_t> second;
            {
                std::lock_guard<std::mutex> lock{mutex};
                if (chunks.size() < 2)
                    return;
                first = util::optional<chunk_t>{chunks.top()};
                chunks.pop();
                second = util::optional<chunk_t>{chunks.top()};
                chunks.pop();
                std::cerr << " Merging " << first->path() << " ("
                     << common::bytes_to_units(first->size())
                     << ") and " << second->path() << " ("
                     << common::bytes_to_units(second->size())
                     << "), " << --remaining << " remaining        \r";
            }
            first->merge_with(*second);
            {
                std::lock_guard<std::mutex> lock{mutex};
                chunks.push(*first);
            }
        }
    };
    for (size_t i = 0; i < thread_ids.size(); ++i) {
        futures.emplace_back(pool.submit_task(task));
    }

    for (auto & fut : futures)
        fut.get();

    std::cerr << std::endl;

    rename(chunks.top().path().c_str(), filename.c_str());

    std::cerr << "Created uncompressed postings file " << filename
              << " (" << common::bytes_to_units(chunks.top().size()) << ")"
              << std::endl;
}

template <class DerivedIndex>
double disk_index<DerivedIndex>::doc_size(doc_id d_id) const
{
    return _doc_sizes->at(d_id);
}

template <class DerivedIndex>
uint64_t disk_index<DerivedIndex>::num_docs() const
{
    return _doc_sizes->size();
}

template <class DerivedIndex>
std::string disk_index<DerivedIndex>::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

template <class DerivedIndex>
std::string disk_index<DerivedIndex>::doc_path(doc_id d_id) const
{
    return _doc_id_mapping.at(d_id);
}

template <class DerivedIndex>
std::vector<doc_id> disk_index<DerivedIndex>::docs() const
{
    std::vector<doc_id> ret(_doc_id_mapping.size());
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::tokenize(corpus::document & doc)
{
    _tokenizer->tokenize(doc);
}

template <class DerivedIndex>
auto disk_index<DerivedIndex>::search_primary(primary_key_type p_id) const
    -> std::shared_ptr<postings_data_type>
{
    uint64_t idx{p_id};

    // if the term doesn't exist in the index, return an empty postings_data
    if(idx >= _term_bit_locations.size())
        return std::make_shared<postings_data_type>(p_id);

    io::compressed_file_reader reader{*_postings, _compression_mapping};
    reader.seek(_term_bit_locations.at(idx));

    auto pdata = std::make_shared<postings_data_type>(p_id);
    pdata->read_compressed(reader);

    return pdata;
}

}
}
