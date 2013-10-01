/**
 * @file pivoted_length.cpp
 * @author Sean Massung
 */

#include "index/ranker/pivoted_length.h"

namespace meta {
namespace index {

pivoted_length::pivoted_length(double s /* = 0.20 */):
    _s{s}
{ /* nothing */ }

double pivoted_length::score_one(const score_data & sd) const
{
    double doc_len = sd.idx.doc_size(sd.d_id);
    double TF = 1 + log(1 + log(sd.doc_term_count));
    double norm = (1 - _s) + _s * (doc_len / sd.avg_dl);
    double IDF = log((sd.num_docs + 1) / sd.idf);

    return TF / norm * sd.query_term_count * IDF;
}

}
}
