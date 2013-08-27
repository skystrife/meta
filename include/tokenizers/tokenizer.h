/**
 * @file tokenizer.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include "meta.h"
#include "cpptoml.h"
#include "index/document.h"
#include "util/invertible_map.h"

namespace meta {
namespace tokenizers {

/**
 * An class that provides a framework to produce tokens. All tokenizers inherit
 * from this class and (possibly) implement tokenize(). This abstract base class
 * does the bookkeeping of mapping term strings to term IDs.
 */
class tokenizer
{
    public:

        /**
         * Constructor. Simply initializes some member variables that
         *  keep track of the term_id mapping.
         */
        tokenizer();

        /**
         * A default virtual destructor.
         */
        virtual ~tokenizer() = default;

        /**
         * Tokenizes a document.
         * @param document document to store the tokenized information in
         */
        void tokenize(index::document & document);

        /**
         * Tokenizes a document
         * @param document document to store tokenized information in
         * @param mapping A function that shows the tokenizer how to convert a
         * string term into its term_id
         */        
        virtual void tokenize_document(index::document & document,
                std::function<term_id(const std::string & term)> mapping) = 0;

        /**
         * Maps terms to term_ids.
         * @param term - the term to check
         * @return the term_id assigned to this term
         */
        virtual term_id mapping(const std::string & term);

        /**
         * Calls the term_id invertible_map's saveMap function.
         * @param filename The filename to save the mapping as
         */
        virtual void save_term_id_mapping(const std::string & filename) const;

        /**
         * Sets the token to termid mapping for this tokenizer.
         * This is useful when reading an inverted index from disk
         *  with an existing mapping.
         * @param filename The filename containing the desired mapping
         */
        virtual void set_term_id_mapping(const std::string & filename);

        /**
         * @return a reference to the structure used to store the termID <->
         * term string mapping
         */
        virtual const util::invertible_map<term_id, std::string> & term_id_mapping() const;

        /**
         * Looks up the actual label that is represented by a term_id.
         * @param termID
         * @return the label
         */
        virtual std::string label(term_id termID) const;

        /**
         * Prints the data associated with this tokenizer, consisting of a term_id and its string
         * value.
         */
        virtual void print_data() const;

        /**
         * @return the number of terms seen so far by this tokenizer
         */
        virtual size_t num_terms() const;

        /**
         * Sets the current termID for this tokenizer. This is useful when running multiple
         * tokenizers on a single documents.
         */
        virtual void set_max_term_id(size_t start);

        /**
         * @return the max term_id associated with this Tokenizer. This is NOT
         * the total number of unique terms seen by this Tokenizer.
         */
        virtual term_id max_term_id() const;

        /**
         * @return a Tokenizer as specified by a config object
         */
        static std::unique_ptr<tokenizer> load_tokenizer(const cpptoml::toml_group & config);

    protected:

        /**
         * Keeps track of the internal mapping of term_ids to strings parsed
         * from the file. This is protected mainly so MultiTokenizer can update
         * its _termMap correctly.
         */
        util::invertible_map<term_id, std::string> _term_map;
        
    private:
 
        /**
         * Internal counter for the number of unique terms seen (used as keys
         * in the invertible_map).
         */
        term_id _current_term_id;

    public:
        /**
         * Basic exception for tokenizer interactions.
         */
        class tokenizer_exception: public std::exception
        {
            public:
                
                tokenizer_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
                std::string _error;
        };

};

}
}

#endif
