/**
 * @file inverted_index.h
 * @author Sean Massung
 */

#include <cstdio>
#include <queue>
#include <iostream>
#include "index/inverted_index.h"
#include "index/chunk.h"

using std::cerr;
using std::endl;

namespace meta {
namespace index {

inverted_index::inverted_index(std::vector<document> & docs,
                               std::shared_ptr<tokenizers::tokenizer> & tok):
    _doc_id_mapping(std::unordered_map<doc_id, std::string>{})
{
    uint32_t num_chunks = tokenize_docs(docs, tok);
    merge_chunks(num_chunks);
    create_lexicon();
    tok->save_term_id_mapping("termids.mapping");
}

uint32_t inverted_index::tokenize_docs(std::vector<document> & docs,
                                   std::shared_ptr<tokenizers::tokenizer> & tok)
{
    std::unordered_map<term_id, postings_data> pdata;
    uint32_t chunk_num = 0;
    uint64_t doc_num = 0;
    for(auto & doc: docs)
    {
        cerr << "[II] Tokenizing " << doc.name() << endl;
        tok->tokenize(doc);
        _doc_id_mapping[doc_num] = doc.name();

        for(auto & f: doc.frequencies())
        {
            postings_data pd{f.first};
            pd.increase_count(doc_num, f.second);
            auto it = pdata.find(f.first);
            if(it == pdata.end())
                pdata.insert(std::make_pair(f.first, pd));
            else
                it->second.merge_with(pd);
        }

        ++doc_num;

        // every k documents, write a chunk
        // TODO: make this based on memory usage instead
        if(doc_num % 100 == 0)
            write_chunk(chunk_num++, pdata);
    }
    
    if(!pdata.empty())
        write_chunk(chunk_num++, pdata);

    return chunk_num;
}

void inverted_index::write_chunk(uint32_t chunk_num,
                                 std::unordered_map<term_id, postings_data> & pdata)
{
    cerr << "[II] Writing chunk " << chunk_num << endl;

    std::vector<std::pair<term_id, postings_data>> sorted{pdata.begin(), pdata.end()};
    std::sort(sorted.begin(), sorted.end());

    std::ofstream outfile{"chunk-" + common::to_string(chunk_num)};
    for(auto & p: sorted)
        outfile << p.second;
    outfile.close();

    pdata.clear();
}

void inverted_index::merge_chunks(uint32_t num_chunks)
{
    // create priority queue of all chunks based on size
    std::priority_queue<chunk> chunks;
    for(uint32_t i = 0; i < num_chunks; ++i)
    {
        std::string filename = "chunk-" + common::to_string(i);
        chunks.push(chunk{filename});
    }

    // merge the smallest two chunks together until there is only one left
    while(chunks.size() > 1)
    {
        chunk first = chunks.top();
        chunks.pop();
        chunk second = chunks.top();
        chunks.pop();

        cerr << "[II] Merging " << first.path() << " (" << first.size()
             << " bytes) and " << second.path() << " (" << second.size()
             << " bytes)" << endl;

        first.merge_with(second);
        chunks.push(first);
    }

    rename(chunks.top().path().c_str(), "postings.index");
    double size = chunks.top().size();
    std::string units = "bytes";

    for(auto & u: {"KB", "MB", "GB", "TB"})
    {
        if(size >= 1024)
        {
            size /= 1024;
            units = u;
        }
    }

    cerr << "[II] Finished creating postings file ("
         << size << " " << units << ")" << endl;
}

void inverted_index::create_lexicon()
{
    // TODO
}

inverted_index::inverted_index(const std::string & index_path)
{
    std::string postings_file = index_path + "/postings.index";
    // TODO
}

uint64_t inverted_index::idf(term_id t_id)
{
    postings_data pdata = search_term(t_id);
    return pdata.idf();
}

uint64_t inverted_index::doc_size(doc_id d_id) const
{
    // TODO
    return d_id;
}

uint64_t inverted_index::term_freq(term_id t_id, doc_id d_id)
{
    postings_data pdata = search_term(t_id);
    return pdata.count(d_id);
}

postings_data inverted_index::search_term(term_id t_id)
{
    // TODO
    return postings_data{t_id};
}

}
}
