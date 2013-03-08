/**
 * @file learn.cpp
 * This creates input for liblinear based on features extracted from my
 *  tokenizers.
 */

#include <vector>
#include <string>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "io/parser.h"
#include "util/common.h"
#include "util/invertible_map.h"

using std::vector;
using std::cout;
using std::endl;
using std::string;

/**
 * Runs the scatterplot creation.
 */
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " configFile" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    string method = config["method"];
    bool quiet = (config["quiet"] == "yes");
    InvertibleMap<string, int> mapping; // for unique ids when printing liblinear data

    int nVal;
    istringstream(config["ngram"]) >> nVal;

    unordered_map<string, NgramTokenizer::NgramType> ngramOpt = {
        {"POS", NgramTokenizer::POS}, {"Word", NgramTokenizer::Word},
        {"FW", NgramTokenizer::FW}, {"Char", NgramTokenizer::Char}
    };

    unordered_map<string, TreeTokenizer::TreeTokenizerType> treeOpt = {
        {"Subtree", TreeTokenizer::Subtree}, {"Depth", TreeTokenizer::Depth},
        {"Branch", TreeTokenizer::Branch}, {"Tag", TreeTokenizer::Tag},
        {"Skeleton", TreeTokenizer::Skeleton}, {"SemiSkeleton", TreeTokenizer::SemiSkeleton},
        {"Multi", TreeTokenizer::Multi}
    };
    
    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);

    size_t done = 0;
    if(method == "ngram")
    {
        Tokenizer* tokenizer = new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]);
        for(size_t i = 0; i < documents.size(); ++i)
        {
            // order of lines in the liblinear input file does NOT matter (tested)
            tokenizer->tokenize(documents[i], NULL);
            cout << documents[i].getLearningData(mapping, false /* using liblinear */);
            if(!quiet && done++ % 20 == 0)
                cerr << "  tokenizing " << static_cast<double>(done) / documents.size() * 100 << "%     \r"; 
        }

        tokenizer->saveTermIDMapping("termidmapping.txt");
        delete tokenizer;
    }
    else if(method == "tree")
    {
        Tokenizer* tokenizer = new TreeTokenizer(treeOpt[config["treeOpt"]]);
        for(size_t i = 0; i < documents.size(); ++i)
        {
            tokenizer->tokenize(documents[i], NULL);
            cout << documents[i].getLearningData(mapping, false /* using liblinear */);
            if(!quiet && done++ % 20 == 0)
                cerr << "  tokenizing " << static_cast<double>(done) / documents.size() * 100 << "%     \r"; 
        }

        delete tokenizer;
    }
    else if(method == "both")
    {
        Tokenizer* treeTokenizer = new TreeTokenizer(treeOpt[config["treeOpt"]]);
        Tokenizer* ngramTokenizer = new NgramTokenizer(nVal, ngramOpt[config["ngramOpt"]]);
        for(size_t i = 0; i < documents.size(); ++i)
        {
            treeTokenizer->tokenize(documents[i], NULL);
            ngramTokenizer->setMaxTermID(treeTokenizer->getNumTerms());
            ngramTokenizer->tokenize(documents[i], NULL);
            treeTokenizer->setMaxTermID(ngramTokenizer->getNumTerms());
            cout << documents[i].getLearningData(mapping, false /* using liblinear */);
            if(!quiet && done++ % 20 == 0)
                cerr << "  tokenizing " << static_cast<double>(done) / documents.size() * 100 << "%     \r"; 
        }
        delete treeTokenizer;
        delete ngramTokenizer;
    }
    else
        cerr << "Method was not able to be determined" << endl;

    if(!quiet)
        cerr << "\r";
}
