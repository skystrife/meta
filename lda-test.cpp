#include <iostream>
#include <string>
#include <vector>

#include "corpus/document.h"
#include "corpus/corpus.h"
#include "stemmers/porter2.h"
#include "tokenizers/ngram/ngram_word_tokenizer.h"
#include "topics/lda_gibbs.h"
#include "topics/parallel_lda_gibbs.h"
#include "topics/lda_cvb.h"

using namespace meta;

int print_usage( const std::string & name ) {
    std::cout << "Usage: " << name << " type alpha beta topics\n"
        "\tRuns LDA of the given type (gibbs, pargibbs, cvb) on the given"
        " corpus, with hyperparameters alpha" " and beta, and topics number"
        " of topics" << std::endl;
    return 1;
}

template <class Model>
int run_lda(const std::unique_ptr<corpus::corpus> & docs, size_t topics, double alpha, double beta ) {
    using tokenizers::tokenizer;

    auto config = cpptoml::parse_file("config.toml");
    std::shared_ptr<tokenizer> tok{tokenizer::load_tokenizer(config)};
    std::vector<corpus::document> tok_docs;
    tok_docs.reserve(docs->size());

    while(docs->has_next())
    {
        corpus::document d{docs->next()};
        common::show_progress(d.id(), docs->size(), 20, "  tokenizing: " );
        tok->tokenize(d);
        tok_docs.emplace_back(d);
    }
    common::end_progress( "  tokenizing: " );

    Model model{ tok_docs, tok, topics, alpha, beta };
    model.run( 1000 );
    model.save( "lda_model" );
    return 0;
}

int run_lda(const std::string & type, double alpha, double beta, size_t topics)
{
    using namespace meta::topics;
    std::cout << "Loading documents...\r" << std::flush;
    auto docs = corpus::corpus::load("config.toml");
    if( type == "gibbs" ) {
        std::cout<< "Beginning LDA using serial Gibbs sampling..." << std::endl;
        return run_lda<lda_gibbs>( docs, topics, alpha, beta );
    } else if( type == "pargibbs" ) {
        std::cout<< "Beginning LDA using parallel Gibbs sampling..." << std::endl;
        return run_lda<parallel_lda_gibbs>( docs, topics, alpha, beta );
    } else if( type == "cvb" ) {
        std::cout<< "Beginning LDA using serial collapsed variational bayes..." << std::endl;
        return run_lda<lda_cvb>( docs, topics, alpha, beta );
    }
    std::cout << "Incorrect method selected: must be gibbs, pargibbs, or cvb" << std::endl;
    return 1;
}

int main( int argc, char ** argv )
{
    if( argc != 5 )
        return print_usage( argv[0] );
    logging::set_cerr_logging();
    std::vector<std::string> args( argv, argv + argc );
    double alpha = std::stod( argv[2] );
    double beta = std::stod( argv[3] );
    size_t topics = std::stoul( argv[4] );
    std::cout << "alpha: " << alpha << "\nbeta: " << beta << "\ntopics: " << topics << std::endl;
    return run_lda( args[1], alpha, beta, topics );
}
