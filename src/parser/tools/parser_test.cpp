/**
 * @file parser_test.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "cpptoml.h"
#include "logging/logger.h"
#include "parser/io/ptb_reader.h"
#include "parser/sr_parser.h"
#include "parser/trees/visitors/sequence_extractor.h"
#include "util/filesystem.h"
#include "util/progress.h"

using namespace meta;

std::string two_digit(uint8_t num)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << static_cast<int>(num);
    return ss.str();
}

int main(int argc, char** argv)
{

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
    {
        LOG(fatal) << "Global configuration must have a prefix key" << ENDLG;
        return 1;
    }

    auto parser_grp = config.get_group("parser");
    if (!parser_grp)
    {
        LOG(fatal) << "Configuration must contain a [parser] group" << ENDLG;
        return 1;
    }

    auto parser_prefix = parser_grp->get_as<std::string>("prefix");
    if (!parser_prefix)
    {
        LOG(fatal) << "[parser] group must contain a prefix to store model files"
                   << ENDLG;
        return 1;
    }

    auto treebank = parser_grp->get_as<std::string>("treebank");
    if (!treebank)
    {
        LOG(fatal) << "[parser] group must contain a treebank path" << ENDLG;
        return 1;
    }

    auto corpus = parser_grp->get_as<std::string>("corpus");
    if (!corpus)
    {
        LOG(fatal) << "[parser] group must contain a corpus" << ENDLG;
        return 1;
    }

    auto train_sections = parser_grp->get_array("test-sections");
    if (!train_sections)
    {
        LOG(fatal) << "[parser] group must contain train-sections" << ENDLG;
        return 1;
    }

    auto section_size = parser_grp->get_as<int64_t>("section-size");
    if (!section_size)
    {
        LOG(fatal) << "[parser] group must contain section-size" << ENDLG;
        return 1;
    }

    std::string path =
        *prefix + "/" + *treebank + "/treebank-3/parsed/mrg/" + *corpus;

    std::vector<sequence::sequence> testing;
    {
        auto begin = train_sections->at(0)->as<int64_t>()->value();
        auto end = train_sections->at(1)->as<int64_t>()->value();
        printing::progress progress(" > Reading testing data: ",
                                    (end - begin + 1) * *section_size);
        for (uint8_t i = begin; i <= end; ++i)
        {
            auto folder = two_digit(i);
            for (uint8_t j = 0; j <= *section_size; ++j)
            {
                progress((i - begin) * 99 + j);
                auto file = *corpus + "_" + folder + two_digit(j) + ".mrg";
                auto filename = path + "/" + folder + "/" + file;
                auto trees = parser::io::extract_trees(filename);
                for (auto & tree : trees)
                {
                    parser::sequence_extractor seq_ex;
                    tree.visit(seq_ex);
                    testing.emplace_back(seq_ex.sequence());
                }
            }
        }
    }
    LOG(info) << testing.size() << " test examples" << ENDLG;

    parser::sr_parser parser{*parser_prefix};

    for (const auto& seq : testing)
    {
        auto tree = parser.parse(seq);
        std::cout << tree << std::endl;
    }

    return 0;
}
