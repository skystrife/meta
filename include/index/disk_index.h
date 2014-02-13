/**
 * @file disk_index.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DISK_INDEX_H_
#define _DISK_INDEX_H_

#include <memory>
#include <vector>
#include "util/pimpl.h"
#include "meta.h"

namespace cpptoml {
class toml_group;
}

namespace meta {

namespace index {
class string_list;
class vocabulary_map;
}

namespace tokenizers {
class tokenizer;
}

namespace util {
template <class>
class disk_vector;
}
}

namespace meta {
namespace index {

/**
 * Holds generic data structures and functions that inverted_index and
 * forward_index both use.
 */
class disk_index
{
  public:
    /**
     * Default destructor.
     */
    virtual ~disk_index() = default;

    /**
     * @return the name of this index.
     */
    std::string index_name() const;

    /**
     * @return the number of documents in this index
     */
    uint64_t num_docs() const;

    /**
     * @param doc_id
     * @return the actual name of this document
     */
    std::string doc_name(doc_id d_id) const;

    /**
     * @param doc_id
     * @return the path to the file containing this document
     */
    std::string doc_path(doc_id d_id) const;

    /**
     * @return a vector of doc_ids that are contained in this index
     */
    std::vector<doc_id> docs() const;

    /**
     * @param d_id The document to search for
     * @return the size of the given document (the total number of terms
     * occurring)
     */
    uint64_t doc_size(doc_id d_id) const;

    /**
     * @param d_id The doc id to find the class label for
     * @return the label of the class that the document belongs to, or an
     * empty string if a label was not assigned
     */
    class_label label(doc_id d_id) const;

    /**
     * @param l_id The id of the class label in question
     * @return the integer label id of a document
     */
    class_label class_label_from_id(label_id l_id) const;

    /**
     * @param d_id
     * @return the number of unique terms in d_id
     */
    virtual uint64_t unique_terms(doc_id d_id) const;

    /**
     * @return the number of unique terms in the index
     */
    virtual uint64_t unique_terms() const;

    /**
     * @param term
     * @return the term_id associated with the parameter
     */
    term_id get_term_id(const std::string & term);

    /**
     * @param t_id The term_id to get the original text for
     * @return the string representation of the term
     */
    std::string term_text(term_id t_id) const;

  protected:
    class disk_index_impl;
    util::pimpl<disk_index_impl> impl_;

    /**
     * Constructor.
     * @param config_file The config settings used to create this index
     * @param name The name of this disk_index
     */
    disk_index(const cpptoml::toml_group & config, const std::string& name);

    /**
     * disk_index may not be copy-constructed.
     */
    disk_index(const disk_index &) = delete;

    /**
     * disk_index may not be copy-assigned.
     */
    disk_index &operator=(const disk_index &) = delete;

  public:

    /**
     * Move constructs a disk_index.
     **/
    disk_index(disk_index&&) = default;

    /**
     * Move assigns a disk_index.
     */
    disk_index& operator=(disk_index&&) = default;
};

}
}

#endif
