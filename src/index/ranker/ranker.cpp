/**
 * @file ranker.cpp
 * @author Sean Massung
 */

#include "corpus/document.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "index/ranker/ranker.h"
#include "index/score_data.h"

namespace meta
{
namespace index
{

std::vector<std::pair<doc_id, double>>
ranker::score(inverted_index& idx, corpus::document& query,
             uint64_t num_results /* = 10 */,
             const std::function<bool(doc_id d_id)>& filter /* = return true */)
{
    if (query.counts().empty())
        idx.tokenize(query);

    score_data sd{idx,            idx.avg_doc_length(),
                  idx.num_docs(), idx.total_corpus_terms(),
                  query};

    // zeros out elements and (if necessary) resizes the vector; this eliminates
    // constructing a new vector each query for the same index
    _results.assign(sd.num_docs, 0.0);

    for (auto& tpair : query.counts())
    {
        term_id t_id{idx.get_term_id(tpair.first)};
        auto pdata = idx.search_primary(t_id);
        sd.doc_count = pdata->counts().size();
        sd.t_id = t_id;
        sd.query_term_count = tpair.second;
        sd.corpus_term_count = idx.total_num_occurences(sd.t_id);
        for (auto& dpair : pdata->counts())
        {
            sd.d_id = dpair.first;
            sd.doc_term_count = dpair.second;
            sd.doc_size = idx.doc_size(dpair.first);
            sd.doc_unique_terms = idx.unique_terms(dpair.first);
            _results[dpair.first] += score_one(sd);
        }
    }

    using doc_pair = std::pair<doc_id, double>;
    auto doc_pair_comp = [](const doc_pair& a, const doc_pair& b)
    { return a.second < b.second; };

    std::priority_queue
        <doc_pair, std::vector<doc_pair>, decltype(doc_pair_comp)> pq{
            doc_pair_comp};
    for (uint64_t id = 0; id < _results.size(); ++id)
    {
        if (!filter(doc_id{id}))
            continue;
        if (!pq.empty() && _results[id] < pq.top().second && pq.size()
                                                             == num_results)
            continue;
        pq.emplace(doc_id{id}, _results[id]);
        if (pq.size() > num_results)
            pq.pop();
    }

    std::vector<doc_pair> sorted;
    while (!pq.empty())
    {
        sorted.emplace_back(pq.top());
        pq.pop();
    }

    return sorted;
}
}
}
