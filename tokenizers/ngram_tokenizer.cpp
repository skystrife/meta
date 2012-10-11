/**
 * @file ngram_tokenizer.cpp
 */

#include <string.h>
#include <cstdlib>
#include "util/common.h"
#include "stemmers/porter2_stemmer.h"
#include "index/document.h"
#include "io/parser.h"
#include "parse_tree.h"
#include "ngram_tokenizer.h"

using std::deque;
using std::unordered_map;
using std::unordered_set;

NgramTokenizer::NgramTokenizer(size_t n, NgramType type):
    _nValue(n), _stopwords(unordered_set<string>())
{
    if(type == Word)
    {
        _extension = ".sen";
        initStopwords();
    }
    else
        _extension = ".pos";
}

void NgramTokenizer::tokenize(Document & document,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{
    Parser parser(document.getPath() + _extension, " \n");

    // initialize the ngram
    deque<string> ngram;
    for(size_t i = 0; i < _nValue && parser.hasNext(); ++i)
    {
        string next = "";
        do
        {
            if(_extension == ".sen")
                next = Porter2Stemmer::stem(Porter2Stemmer::trim(parser.next()));
            else
                next = parser.next();
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the rest of the ngrams
    while(parser.hasNext())
    {
        string wordified = wordify(ngram);
        document.increment(getMapping(wordified), 1, docFreq);
        ngram.pop_front();
        string next = "";
        do
        {
            if(_extension == ".sen")
                next = Porter2Stemmer::stem(Porter2Stemmer::trim(parser.next()));
            else
                next = parser.next();
        } while(_stopwords.find(next) != _stopwords.end() && parser.hasNext());
        ngram.push_back(next);
    }

    // add the last token
    document.increment(getMapping(wordify(ngram)), 1, docFreq);
}

void NgramTokenizer::initStopwords()
{
    Parser parser("data/lemur-stopwords.txt", "\n");
    while(parser.hasNext())
        _stopwords.insert(Porter2Stemmer::stem(parser.next()));
}

size_t NgramTokenizer::getNValue() const
{
    return _nValue;
}

string NgramTokenizer::wordify(const deque<string> & words) const
{
    string result = "";
    for(auto & word: words)
        result += (word + " ");
    result = result.substr(0, result.size() - 1);
    return result;
}

string NgramTokenizer::setLower(const string & original) const
{
    string word = "";
    for(auto ch: original)
    {
        if(ch > 'A' && ch < 'Z')
            ch += 32;
    }
    return word;
}
