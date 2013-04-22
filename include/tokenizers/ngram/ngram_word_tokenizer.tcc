/**
 * @file ngram_word_tokenizer.tcc
 */

#include "io/parser.h"
#include "io/config_reader.h"
#include "tokenizers/ngram/ngram_word_tokenizer.h"

namespace meta {
namespace tokenizers {

using std::deque;
using std::string;
using std::unordered_map;
using std::unordered_set;

using index::document;
using io::Parser;

template <class Stemmer>
ngram_word_tokenizer<Stemmer>::ngram_word_tokenizer(size_t n, ngram_word_traits::StopwordType stopwordType):
    ngram_tokenizer(n),
    _stopwords(unordered_set<string>())
{
    if(stopwordType != ngram_word_traits::NoStopwords)
        init_stopwords();
}

template <class Stemmer>
void ngram_word_tokenizer<Stemmer>::tokenize_document(document & document,
        std::function<term_id(const std::string &)> mapping,
        const std::shared_ptr<unordered_map<term_id, unsigned int>> & docFreq)
{
    Parser parser(document.path() + ".sen", " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < n_value() && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            next = stem(parser.next());
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(mapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "";
        do
        {
            next = stem(parser.next());
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(mapping(wordify(ngram)), 1, docFreq);
}

template <class Stemmer>
void ngram_word_tokenizer<Stemmer>::init_stopwords()
{
    auto config = io::config_reader::read("config.ini");
    Parser parser(config["stop-words"], "\n");
    while(parser.hasNext())
        _stopwords.insert(stem(parser.next()));
}

}
}
