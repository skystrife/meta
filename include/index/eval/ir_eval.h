/**
 * @file ir_eval.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_IR_EVAL_H_
#define _META_IR_EVAL_H_

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "meta.h"

namespace meta
{
namespace index
{

/**
 * Evaluates lists of ranked documents returned from a search engine; can give
 * stats per-query (e.g. precision) or over a series of queries (e.g. MAP).
 */
class ir_eval
{
  public:
    using result_type = std::vector<std::pair<doc_id, double>>;

    /**
     * @param config_file Path to cpptoml configuration file
     */
    ir_eval(const std::string& config_file);

    /**
     * @param results The ranked list of results
     * @param q_id The query that was run to produce these results
     * @param num_docs For p@num_docs
     * @return the precision: \f$ \frac{\# relevant~retrieved~docs}{\#
     * retrieved~docs} \f$
     */
    double precision(const result_type& results, query_id q_id,
                     uint64_t num_docs = std::numeric_limits<uint64_t>::max()
    ) const;

    /**
     * @param results The ranked list of results
     * @param q_id The query that was run to produce these results
     * @param num_docs For r@num_docs
     * @return the recall: \f$ \frac{\# relevant~retrieved~docs}{\#
     * relevant~docs} \f$
     */
    double recall(const result_type& results, query_id q_id,
                  uint64_t num_docs = std::numeric_limits<uint64_t>::max()
    ) const;

    /**
     * @param results The ranked list of results
     * @param q_id The query that was run to produce these results
     * @param num_docs For f1@num_docs
     * @param beta Attach beta times as much importance to recall compared to
     * precision (default 1.0, or equal)
     * @return the F1 score: \f$ \frac{(1+\beta^2)(P\cdot R)}{(\beta^2\cdot P
     * )+R} \f$
     */
    double f1(const result_type& results, query_id q_id,
              uint64_t num_docs = std::numeric_limits<uint64_t>::max(),
              double beta = 1.0) const;

    /**
     * @param results The ranked list of results
     * @param q_id The query that was run to produce these results
     * @param out The stream to print to
     */
    void print_stats(const result_type& results, query_id q_id,
                     std::ostream& out = std::cout) const;

  private:
    /**
     * query_id -> (doc_id -> relevance) mapping
     * If the doc_id isn't in the map, it is non-relevant.
     */
    std::unordered_map<query_id, std::unordered_map<doc_id, uint8_t>> _qrels;

    /**
     * Initializes the _byte_index member with pointers into the _judgements
     * file
     */
    void init_index(const std::string& path);

    /**
     * @param results The ranked list of results
     * @param q_id The query that was run to produce these results
     * @param num_docs For @num_docs scores
     * @return the number of relevant results that were retrieved
     */
    double relevant_retrieved(const result_type& results, query_id q_id,
                              uint64_t num_docs) const;

  public:
    /**
     * Basic exception for ir_eval interactions.
     */
    class ir_eval_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};
}
}

#endif
