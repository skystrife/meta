/**
 * @file information_gain.cpp
 * @author Sean Massung
 */

#include "features/information_gain.h"

namespace meta
{
namespace features
{

const std::string information_gain::id = "info-gain";

double information_gain::score(label_id lid, term_id tid) const
{
    double p_tc = term_and_class(tid, lid);
    double p_ntnc = not_term_and_not_class(tid, lid);
    double p_ntc = not_term_and_class(tid, lid);
    double p_tnc = term_and_not_class(tid, lid);
    double p_c = prob_class(lid);
    double p_t = prob_term(tid);
    double p_nc = 1.0 - p_c;
    double p_nt = 1.0 - p_t;

    double gain_tc = p_tc * std::log(p_tc / (p_t * p_c));
    double gain_ntnc = p_ntnc * std::log(p_ntnc / (p_nt * p_nc));
    double gain_ntc = p_ntc * std::log(p_ntc / (p_nt * p_c));
    double gain_tnc = p_tnc * std::log(p_tnc / (p_t * p_nc));

    // if any denominators were zero, make the expression zero
    if (std::isnan(gain_tc))
        gain_tc = 0.0;
    if (std::isnan(gain_ntnc))
        gain_ntnc = 0.0;
    if (std::isnan(gain_ntc))
        gain_ntc = 0.0;
    if (std::isnan(gain_tnc))
        gain_tnc = 0.0;

    return gain_tc + gain_ntnc + gain_ntc + gain_tnc;
}
}
}
