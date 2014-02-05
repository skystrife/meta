/**
 * @file lda_cvb.cpp
 */

#include <random>
#include "index/postings_data.h"
#include "topics/lda_cvb.h"

namespace meta
{
namespace topics
{

lda_cvb::lda_cvb(index::forward_index& idx, uint64_t num_topics, double alpha,
                 double beta)
    : lda_model{idx, num_topics}, alpha_{alpha}, beta_{beta}
{
    /* nothing */
}

void lda_cvb::run(uint64_t num_iters, double convergence)
{
    std::cerr << "Running LDA inference...\n";
    initialize();
    for (size_t i = 0; i < num_iters; ++i)
    {
        std::cerr << "Iteration " << i + 1 << "/" << num_iters << ":\r";
        double max_change = perform_iteration();
        std::cerr << "\t\t\t\t\t\tmaximum change in gamma: " << max_change
                  << "    \r";
        if (max_change <= convergence)
        {
            std::cerr << "\nFound convergence after " << i + 1
                      << " iterations!\n";
            break;
        }
    }
    std::cerr << "\nFinished maximum iterations, or found convergence!\n";
}

void lda_cvb::initialize()
{
    std::random_device rdev;
    std::mt19937 rng(rdev());
    std::cerr << "Initalizing::\r";
    for (doc_id i{0}; i < idx_.num_docs(); ++i)
    {
        for (auto& freq : idx_.search_primary(i)->counts())
        {
            double sum = 0;
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                double random = rng();
                gamma_[i][freq.first][k] = random;
                sum += random;
            }
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma_[i][freq.first][k] /= sum;
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k] += contrib;
                topic_term_mean_[k][freq.first] += contrib;
                topic_mean_[k] += contrib;
            }
        }
    }
}

double lda_cvb::perform_iteration()
{
    double max_change = 0;
    for (doc_id i{0}; i < idx_.num_docs(); ++i)
    {
        for (auto& freq : idx_.search_primary(i)->counts())
        {
            // remove this word occurrence from means
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k] -= contrib;
                topic_term_mean_[k][freq.first] -= contrib;
                topic_mean_[k] -= contrib;
            }
            double min = 0;
            double max = 0;
            std::unordered_map<topic_id, double> old_gammas =
                gamma_[i][freq.first];
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                // recompute gamma using CVB0 formula
                gamma_[i][freq.first][k] =
                    compute_term_topic_probability(freq.first, k) *
                    doc_topic_mean_.at(i).at(k);
                min = std::min(min, gamma_[i][freq.first][k]);
                max = std::max(max, gamma_[i][freq.first][k]);
            }
            // normalize gamma and update means
            for (topic_id k{0}; k < num_topics_; ++k)
            {
                gamma_[i][freq.first][k] =
                    (gamma_[i][freq.first][k] - min) / (max - min);
                double contrib = freq.second * gamma_[i][freq.first][k];
                doc_topic_mean_[i][k] += contrib;
                topic_term_mean_[k][freq.first] += contrib;
                topic_mean_[k] += contrib;
                max_change =
                    std::max(max_change, std::abs(old_gammas[k] -
                                                  gamma_[i][freq.first][k]));
            }
        }
    }
    return max_change;
}

double lda_cvb::compute_term_topic_probability(term_id term,
                                               topic_id topic) const
{
    return (topic_term_mean_.at(topic).at(term) + beta_) /
           (topic_mean_.at(topic) + num_words_ * beta_);
}

double lda_cvb::compute_doc_topic_probability(doc_id doc, topic_id topic) const
{
    return (doc_topic_mean_.at(doc).at(topic) + alpha_) /
           (idx_.doc_size(doc) + num_topics_ * alpha_);
}
}
}
