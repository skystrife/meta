/**
 * @file correlation_coefficient.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CORRELATION_COEFFICIENT_H_
#define META_CORRELATION_COEFFICIENT_H_

#include "features/feature_selector.h"

namespace meta
{
namespace features
{
/**
 * Performs correlation coefficient feature selection:
 * \f$ \chi^2(t, c_i) =
 * \frac{(P(t,c_i) P(\overline{t}, \overline{c_i}) - P(t, \overline{c_i})
 * P(\overline{t},c_i))^2}
 *  {P(t) P(\overline{t}) P(c_i) P(\overline{c_i})} \f$
 */
class correlation_coefficient : public feature_selector
{
  public:
    /// Inherit constructor.
    using feature_selector::feature_selector;

    /// Identifier for this feature_selector.
    const static std::string id;

    /**
     * Scores the (label_id, term) pair according to this feature selection
     * metric.
     * @param lid
     * @param tid
     */
    virtual double score(label_id lid, term_id tid) const override;
};
}
}
#endif
