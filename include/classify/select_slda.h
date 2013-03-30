/**
 * @file select_slda.h
 */

#ifndef _SELECT_SLDA_H_
#define _SELECT_SLDA_H_

#include <vector>
#include <utility>
#include "index/document.h"

namespace meta {
namespace classify {
namespace feature_select {

    /**
     * Creates an sLDA input file from the given documents and runs the
     * supervised latent Dirichlet topic modeling algorithm on the data.
     * Features are sorted by weight, and then returned in order in a vector
     * like the other feature selection algorithms.
     * @param docs The documents to extract features from
     * @return a vector of TermIDs sorted by importance
     */
    std::vector<std::pair<index::TermID, double>> slda(const std::vector<index::Document> & docs);

}
}
}

#endif
