/**
 * @file naive_bayes.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _NAIVE_BAYES_H_
#define _NAIVE_BAYES_H_

#include <unordered_map>
#include "index/forward_index.h"
#include "classify/classifier/classifier.h"
#include "meta.h"

namespace meta {
namespace classify {

/**
 * Implements the Naive Bayes classifier, a simplistic probabilistic classifier
 * that uses Bayes' theorem with strong feature independence assumptions.
 */
class naive_bayes: public classifier
{
    public:

        /**
         * Constructor: learns class models based on a collection of training
         * documents.
         * @param idx The index to run the classifier on
         * @param alpha Optional smoothing parameter for term frequencies
         * @param beta Optional smoothing parameter for class frequencies
         */
        naive_bayes(index::forward_index & idx,
                    double alpha = 0.1, double beta = 0.1);

        /**
         * Creates a classification model based on training documents.
         * Calculates P(term|class) and P(class) for all the training documents.
         * @param docs The training documents
         */
        void train(const std::vector<doc_id> & docs) override;

        /**
         * Classifies a document into a specific group, as determined by
         * training data.
         * @param d_id The document to classify
         * @return the class it belongs to
         */
        class_label classify(doc_id d_id) override;

        /**
         * Resets any learning information associated with this classifier.
         */
        void reset() override;

    private:

        /**
         * Contains P(term|class) for each class.
         */
        std::unordered_map<
            class_label,
            std::unordered_map<term_id, double>
        > _term_probs;

        /**
         * Contains the number of documents in each class
         */
        std::unordered_map<class_label, size_t> _class_counts;

        /** The number of training documents */
        size_t _total_docs;

        /** smoothing parameter for term counts */
        const double _alpha;

        /** smoothing parameter for class counts */
        const double _beta;
};

}
}

#endif
