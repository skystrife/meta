/**
 * @file forward_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _FORWARD_INDEX_H_
#define _FORWARD_INDEX_H_

#include <string>
#include <vector>
#include <memory>
#include "index/postings_data.h"
#include "corpus/document.h"
#include "index/make_index.h"
#include "meta.h"

namespace meta {
namespace index {

/**
 * The forward_index stores information on a corpus by doc_ids.  Each doc_id key
 * is associated with a distribution of term_ids or term "counts" that occur in
 * that particular document.
 */
class forward_index
{
   public:
    class forward_index_exception;

    using primary_key_type   = doc_id;
    using secondary_key_type = term_id;
    using postings_data_type = postings_data<doc_id, term_id>;
    using exception = forward_index_exception;

   protected:
    /**
     * @param config The toml_group that specifies how to create the
     * index.
     */
    forward_index(const cpptoml::toml_group &config);

   public:
    /**
     * Move constructs a forward_index.
     * @param other The forward_index to move into this one.
     */
    forward_index(forward_index &&other) = default;

    /**
     * Move assigns a forward_index.
     * @param other The forward_index to move into this one.
     */
    forward_index &operator=(forward_index &&other) = default;

    /**
     * forward_index may not be copy-constructed.
     */
    forward_index(const forward_index &) = delete;

    /**
     * forward_index may not be copy-assigned.
     */
    forward_index &operator=(const forward_index &) = delete;

    /**
     * Default destructor.
     */
    virtual ~forward_index() = default;

    /**
     * @param d_id The doc id to find the class label for
     * @return the label of the class that the document belongs to, or an empty
     * string if a label was not assigned
     */
    class_label label(doc_id d_id) const;

    /**
     * @param l_id The id of the class label in question
     * @return the integer label id of a document
     */
    class_label class_label_from_id(label_id l_id) const;

    /**
     * @param d_id The doc_id to search for
     * @return the postings data for a given doc_id
     */
    virtual std::shared_ptr<postings_data_type>
        search_primary(doc_id d_id) const;

    /**
     * @param d_id The document id of the doc to convert to liblinear format
     * @return the string representation liblinear format
     */
    std::string liblinear_data(doc_id d_id) const;

    /**
     * @return the number of documents in this index
     */
    uint64_t num_docs() const;

    /**
     * @return a vector of doc_ids that are contained in this index
     */
    std::vector<doc_id> docs() const;

    /**
     * @return the number of unique terms in the index
     */
    uint64_t unique_terms() const;

    /**
     * forward_index is a friend of the factory method used to create
     * it.
     */
    friend forward_index make_index<forward_index>(const std::string &);

   private:

    /**
     * This function loads a disk index from its filesystem
     * representation.
     */
    void load_index();

    /**
     * This function initializes the forward index.
     * @param config_file The configuration file used to create the index
     */
    void create_index(const std::string & config_file);

    /**
     * @param config the configuration settings for this index
     */
    void create_libsvm_postings(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     */
    void create_libsvm_metadata(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     */
    void uninvert(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     */
    void create_univerted_metadata(const cpptoml::toml_group& config);

    /**
     * @param config the configuration settings for this index
     * @return whether this index will be based off of a single
     * libsvm-formatted corpus file
     */
    bool is_libsvm_format(const cpptoml::toml_group& config) const;

    /** the directory name for this index on disk */
    std::string _index_name;

    public:
        /**
         * Basic exception for forward_index interactions.
         */
        class forward_index_exception: public std::exception
        {
            public:

                forward_index_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what() const throw()
                {
                    return _error.c_str();
                }

            private:
                std::string _error;
        };

        /**
         * Factory method for creating indexes.
         */
        template <class Index, class... Args>
        friend Index make_index(const std::string & config_file,
                                Args &&... args);

        /**
         * Factory method for creating indexes that are cached.
         */
        template <class Index,
                  template <class, class> class Cache,
                  class... Args>
        friend cached_index<Index, Cache>
        make_index(const std::string & config_file, Args &&... args);
};

}
}

#endif
