/**
 * @file select_info_gain.cpp
 */

#include "classify/select_info_gain.h"
#include "util/common.h"
#include "parallel/parallel_for.h"

namespace meta {
namespace classify {

using std::vector;
using std::unordered_set;
using std::unordered_map;
using std::pair;
using std::string;

using index::Document;
using index::TermID;

vector<pair<TermID, double>> select_info_gain::select(const vector<Document> & docs)
{
    unordered_set<string> classes(get_class_space(docs));
    unordered_set<TermID> terms(get_term_space(docs));
    unordered_map<string, vector<Document>> buckets(partition_classes(docs));
    unordered_map<TermID, double> feature_weights;

    size_t total_terms = 0;
    for(auto & p: buckets)
        for(auto & d: p.second)
            total_terms += d.getLength();

    std::mutex _mutex;
    for(auto & c: classes)
    {
        string progress = "  " + c + ": ";
        size_t i = 0;
        parallel::parallel_for(terms.begin(), terms.end(), [&](const TermID t)
        {
            double gain = calc_info_gain(t, c, docs.size(), total_terms, buckets);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                common::show_progress(i++, terms.size(), 50, progress);
                if(feature_weights[t] < gain)
                    feature_weights[t] = gain;
            }
        });
        common::end_progress(progress);
    }

    vector<pair<TermID, double>> features(feature_weights.begin(), feature_weights.end());
    std::sort(features.begin(), features.end(),
        [](const pair<TermID, double> & a, const pair<TermID, double> & b) {
            return a.second > b.second;
        }
    );

    return features;

}

double select_info_gain::calc_info_gain(TermID termID, const string & label,
        size_t total_docs, size_t total_terms, const unordered_map<string, vector<Document>> & classes)
{
    double term_count = 0;
    for(auto & cls: classes)
    {
        for(auto & d: cls.second)
        {
            auto freqs = d.getFrequencies();
            auto it = freqs.find(termID);
            if(it != freqs.end())
                term_count += it->second;
        }
    }

    double p_tc = term_given_class(termID, label, classes);
    double p_ntnc = not_term_given_not_class(termID, label, classes);
    double p_ntc = 1.0 - p_tc;
    double p_tnc = 1.0 - p_ntnc;
    double p_c = static_cast<double>(classes.at(label).size()) / total_docs;
    double p_t = static_cast<double>(term_count) / total_terms;
    double p_nc = 1.0 - p_c;
    double p_nt = 1.0 - p_t;

    double gain_tc = p_tc * log(p_tc / (p_t * p_c));
    double gain_ntnc = p_ntnc * log(p_ntnc / (p_nt * p_nc));
    double gain_ntc = p_ntc * log(p_ntc / (p_nt * p_c));
    double gain_tnc = p_tnc * log(p_tnc / (p_t * p_nc));

    return gain_tc + gain_ntnc + gain_ntc + gain_tnc;
}

}
}
