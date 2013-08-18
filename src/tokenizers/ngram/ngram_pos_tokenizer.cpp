/**
 * @file ngram_pos_tokenizer.cpp
 */

#include <unordered_map>
#include "io/parser.h"
#include "tokenizers/ngram/ngram_pos_tokenizer.h"

namespace meta {
namespace tokenizers {

ngram_pos_tokenizer::ngram_pos_tokenizer(size_t n):
    ngram_simple_tokenizer(n) { /* nothing */ }

void ngram_pos_tokenizer::tokenize_document(index::document & document,
        std::function<term_id(const std::string &)> mapping,
        const std::shared_ptr<std::unordered_map<term_id, uint64_t>> & docFreq)
{
    io::parser parser(document.path() + ".pos", " \n");
    simple_tokenize(parser, document, mapping, docFreq);
}

}
}
