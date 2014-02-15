/**
 * @file postings_data.tcc
 * @author Sean Massung
 */

#include <algorithm>
#include <cstring>
#include "index/postings_data.h"

namespace meta {
namespace index {

template <class PrimaryKey, class SecondaryKey>
postings_data<PrimaryKey, SecondaryKey>::postings_data(PrimaryKey p_id):
    _p_id{p_id}
{ /* nothing */ }

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::merge_with(
        postings_data & other)
{
    auto searcher = [](const pair_t & p, const SecondaryKey & s) { return p.first < s;
    };

    // O(n log n) now, could be O(n)

    // if the primary_key doesn't exist, add onto back
    uint64_t orig_length = _counts.size();
    for(auto & p: other._counts)
    {
        auto it = std::lower_bound(_counts.begin(),
                                   _counts.begin() + orig_length,
                                   p.first,
                                   searcher);
        if(it == _counts.end() || it->first != p.first)
            _counts.emplace_back(std::move(p));
        else
            it->second += p.second;
    }

    // sort _counts again to fix new elements added onto back
    if (_counts.size() > orig_length) {
        std::sort(_counts.begin(), _counts.end(),
            [](const pair_t & a, const pair_t & b) {
                return a.first < b.first;
            }
        );
    }
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::increase_count(
        SecondaryKey s_id, double amount)
{
    auto it = std::lower_bound(_counts.begin(), _counts.end(), s_id,
        [](const pair_t & p, const SecondaryKey & s) {
            return p.first < s;
        }
    );

    if(it == _counts.end())
        _counts.emplace_back(s_id, amount);
    else if(it->first != s_id)
        _counts.emplace(it, s_id, amount);
    else
        it->second += amount;
}

template <class PrimaryKey, class SecondaryKey>
double postings_data<PrimaryKey, SecondaryKey>::count(SecondaryKey s_id) const
{
    auto it = std::lower_bound(_counts.begin(), _counts.end(), s_id,
        [](const pair_t & p, const SecondaryKey & s) {
            return p.first < s;
        }
    );

    if(it == _counts.end() || it->first != s_id)
        return 0.0;
    return it->second;
}

template <class PrimaryKey, class SecondaryKey>
const std::vector<std::pair<SecondaryKey, double>> &
postings_data<PrimaryKey, SecondaryKey>::counts() const
{
    return _counts;
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::set_counts(const count_t & counts)
{
    _counts = counts;
    std::sort(_counts.begin(), _counts.end(),
        [](const pair_t & a, const pair_t & b) {
            return a.first < b.first;
        }
    );
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::set_primary_key(PrimaryKey new_key)
{
    _p_id = new_key;
}

template <class PrimaryKey, class SecondaryKey>
bool postings_data<PrimaryKey, SecondaryKey>::operator<(const postings_data & other) const
{
    return primary_key() < other.primary_key();
}

template <class PrimaryKey, class SecondaryKey>
bool operator==(const postings_data<PrimaryKey, SecondaryKey> & lhs,
                const postings_data<PrimaryKey, SecondaryKey> & rhs) {
    return lhs.primary_key() == rhs.primary_key();
}

template <class PrimaryKey, class SecondaryKey>
PrimaryKey postings_data<PrimaryKey, SecondaryKey>::primary_key() const
{
    return _p_id;
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::write_compressed(
        io::compressed_file_writer & writer) const
{
    count_t mutable_counts{_counts};
    writer.write(mutable_counts[0].first);
    if(std::is_same<PrimaryKey, term_id>::value
       || std::is_same<PrimaryKey, std::string>::value)
        writer.write(static_cast<uint64_t>(mutable_counts[0].second));
    else
    {
        uint64_t to_write;
        std::memcpy(&to_write, &mutable_counts[0].second,
                    sizeof(mutable_counts[0].second));
        writer.write(to_write);
    }

    // use gap encoding on the SecondaryKeys (we know they are integral types)
    uint64_t cur_id = mutable_counts[0].first;
    for(size_t i = 1; i < mutable_counts.size(); ++i)
    {
        uint64_t temp_id = mutable_counts[i].first;
        mutable_counts[i].first = mutable_counts[i].first - cur_id;
        cur_id = temp_id;

        writer.write(mutable_counts[i].first);
        if(std::is_same<PrimaryKey, term_id>::value
           || std::is_same<PrimaryKey, std::string>::value)
            writer.write(static_cast<uint64_t>(mutable_counts[i].second));
        else
        {
            uint64_t to_write;
            std::memcpy(&to_write, &mutable_counts[i].second,
                        sizeof(mutable_counts[i].second));
            writer.write(to_write);
        }
    }

    // mark end of postings_data
    writer.write(_delimiter);
}

template <class PrimaryKey, class SecondaryKey>
void postings_data<PrimaryKey, SecondaryKey>::read_compressed(
        io::compressed_file_reader & reader)
{
    _counts.clear();
    uint64_t last_id = 0;

    while(true)
    {
        uint64_t this_id = reader.next();

        // have we reached a delimiter?
        if(this_id == _delimiter)
            break;

        // we're using gap encoding
        last_id += this_id;
        SecondaryKey key{last_id};
        uint64_t next = reader.next();
        double count;
        if(std::is_same<PrimaryKey, term_id>::value)
            count = static_cast<double>(next);
        else
            std::memcpy(&count, &next, sizeof(next));

        _counts.emplace_back(key, count);
    }

    // compress vector to conserve memory (it shouldn't be modified again after
    // this)
    _counts.shrink_to_fit();
}

namespace {
    template <class T>
    uint64_t length(const T & elem,
                    typename std::enable_if<
                        std::is_same<T, std::string>::value
                    >::type * = nullptr) {
        return elem.size();
    }

    template <class T>
    uint64_t length(const T & elem,
                    typename std::enable_if<
                        !std::is_same<T, std::string>::value
                    >::type * = nullptr) {
        return sizeof(elem);
    }
}

template <class PrimaryKey, class SecondaryKey>
uint64_t postings_data<PrimaryKey, SecondaryKey>::bytes_used() const
{
    return sizeof(pair_t) * _counts.size() + length(_p_id);
}

}
}
