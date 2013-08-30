/**
 * @file lm_ranker.cpp
 * @author Sean Massung
 */

#include "index/ranker/lm_ranker.h"

namespace meta {
namespace index {

double language_model_ranker::score_one(inverted_index & idx,
    const document & query,
    const std::pair<term_id, uint64_t> & tpair,
    const std::pair<doc_id, uint64_t> & dpair,
    uint64_t unique_terms) const
{
    double ps = smoothed_prob(idx, tpair.first, dpair.first);
    double doc_const = doc_constant(idx, dpair.first);
    double pc = 0.0; // TODO

    return ps / doc_const * pc + query.length() * log(doc_const);
}

}
}
