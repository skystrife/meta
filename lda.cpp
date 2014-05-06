#include <iostream>
#include <string>
#include <vector>

#include "topics/lda_gibbs.h"
#include "topics/parallel_lda_gibbs.h"
#include "topics/lda_cvb.h"
#include "topics/lda_scvb.h"

#include "cpptoml.h"

#include "caching/no_evict_cache.h"
#include "index/forward_index.h"
#include "logging/logger.h"

using namespace meta;

template <class Gibbs, class Index>
int run_lda(Index& idx, uint64_t num_iters, uint64_t burn_in, uint64_t topics,
            double alpha, double beta, const std::string& save_prefix)
{
    Gibbs model{idx, topics, alpha, beta, burn_in};
    model.run(num_iters);
    model.save(save_prefix);
    model.save_topic_assignments(save_prefix + ".assignments");
    return 0;
}

template <class Model, class Index>
int run_lda(Index& idx, uint64_t num_iters, uint64_t topics, double alpha,
            double beta, const std::string& save_prefix)
{
    Model model{idx, topics, alpha, beta};
    model.run(num_iters);
    model.save(save_prefix);
    return 0;
}

bool check_parameter(const std::string& file, const cpptoml::toml_group& group,
                     const std::string& param)
{
    if (!group.contains(param))
    {
        std::cerr << "Missing lda configuration parameter " << param << " in "
                  << file << std::endl;
        return false;
    }
    return true;
}

int run_lda(const std::string& config_file)
{
    using namespace meta::topics;
    auto config = cpptoml::parse_file(config_file);

    if (!config.contains("lda"))
    {
        std::cerr << "Missing lda configuration group in " << config_file
                  << std::endl;
        return 1;
    }

    auto lda_group = config.get_group("lda");

    if (!check_parameter(config_file, *lda_group, "alpha") ||
        !check_parameter(config_file, *lda_group, "beta") ||
        !check_parameter(config_file, *lda_group, "topics") ||
        !check_parameter(config_file, *lda_group, "inference") ||
        !check_parameter(config_file, *lda_group, "max-iters") ||
        !check_parameter(config_file, *lda_group, "model-prefix"))
        return 1;

    auto type = *lda_group->get_as<std::string>("inference");
    uint64_t iters = *lda_group->get_as<int64_t>("max-iters");
    auto alpha = *lda_group->get_as<double>("alpha");
    auto beta = *lda_group->get_as<double>("beta");
    uint64_t topics = *lda_group->get_as<int64_t>("topics");
    auto save_prefix = *lda_group->get_as<std::string>("model-prefix");

    auto f_idx =
        index::make_index<index::forward_index, caching::no_evict_cache>(
            config_file);
    if (type == "gibbs")
    {
        if (!check_parameter(config_file, *lda_group, "burn-in"))
            return 1;
        uint64_t burn_in = *lda_group->get_as<int64_t>("burn-in");
        std::cout << "Beginning LDA using serial Gibbs sampling..."
                  << std::endl;
        return run_lda<lda_gibbs>(f_idx, iters, burn_in, topics, alpha, beta,
                                  save_prefix);
    }
    else if (type == "pargibbs")
    {
        if (!check_parameter(config_file, *lda_group, "burn-in"))
            return 1;
        uint64_t burn_in = *lda_group->get_as<int64_t>("burn-in");
        std::cout << "Beginning LDA using parallel Gibbs sampling..."
                  << std::endl;
        return run_lda<parallel_lda_gibbs>(f_idx, iters, burn_in, topics, alpha,
                                           beta, save_prefix);
    }
    else if (type == "cvb")
    {
        std::cout << "Beginning LDA using serial collapsed variational bayes..."
                  << std::endl;
        return run_lda<lda_cvb>(f_idx, iters, topics, alpha, beta, save_prefix);
    }
    else if (type == "scvb")
    {
        std::cout
            << "Beginning LDA using stochastic collapsed variational bayes..."
            << std::endl;
        return run_lda<lda_scvb>(f_idx, iters, topics, alpha, beta,
                                 save_prefix);
    }
    std::cout << "Incorrect method selected: must be gibbs, pargibbs, or cvb"
              << std::endl;
    return 1;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();
    return run_lda(argv[1]);
}
