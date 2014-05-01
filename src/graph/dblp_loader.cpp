/**
 * @file dblp_loader.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "graph/dblp_loader.h"

namespace meta
{
namespace graph
{
namespace dblp_loader
{
void load(graph::directed_graph<graph::dblp_node>& g, const std::string& prefix,
          uint64_t start_year /* = 0 */,
          uint64_t end_year /* = std::numeric_limits<uint64_t>::max() */)
{
    // load nodes

    io::parser authors{prefix + "authors.txt", "\t\n"};
    while (authors.has_next())
        g.insert(graph::dblp_node{"author", authors.next()});

    node_id venue_offset{g.size() - 1}; // -1 because DBLP data is 1-based

    io::parser venues{prefix + "venues.txt", "\t\n"};
    while (venues.has_next())
        g.insert(graph::dblp_node{"venue", venues.next()});

    node_id term_offset{g.size() - 1};

    io::parser terms{prefix + "terms.txt", "\n"};
    while (terms.has_next())
        g.insert(graph::dblp_node{"term", terms.next()});

    node_id paper_offset{g.size() - 1}; // -1 because DBLP data is 1-based

    std::vector<node_id> node_map;
    node_map.push_back(node_id{0});
    io::parser papers{prefix + "papers.txt", "\t\n"};
    uint64_t year;
    uint64_t line = 1;
    while (papers.has_next())
    {
        std::istringstream year_stream{papers.next()};
        year_stream >> year;
        std::string title = papers.next();
        if (year < start_year || year > end_year)
            node_map.push_back(node_id{0});
        else
        {
            g.insert(graph::dblp_node{"paper", title});
            node_map.push_back(node_id{line + paper_offset});
            ++line;
        }
    }

    // load edges

    node_id source;
    node_id dest;

    std::ifstream paper_paper{prefix + "paper_paper.txt"};
    while (paper_paper >> source >> dest)
    {
        if (node_map[source] != 0 && node_map[dest] != 0)
            g.add_edge(node_map[source], node_map[dest]);
    }

    std::ifstream paper_author{prefix + "paper_author.txt"};
    while (paper_author >> source >> dest)
    {
        try
        {
            if (node_map[source] != 0)
            {
                g.add_edge(node_map[source], node_id{dest - 1});
                g.add_edge(node_id{dest - 1}, node_map[source]);
            }
        }
        catch (graph::directed_graph_exception& e)
        {
            // it seems there is one duplicate edge in the DBLP data
            std::cout << " > Warning: attempted to add duplicate edge: "
                      << source << " " << dest << std::endl;
        }
    }

    std::ifstream paper_venue{prefix + "paper_venue.txt"};
    while (paper_venue >> source >> dest)
    {
        if (node_map[source] != 0)
        {
            g.add_edge(node_map[source], dest + venue_offset);
            g.add_edge(dest + venue_offset, node_map[source]);
        }
    }

    std::ifstream paper_term{prefix + "paper_term.txt"};
    while (paper_term >> source >> dest)
    {
        if (node_map[source] != 0)
        {
            g.add_edge(node_map[source], dest + term_offset);
            g.add_edge(dest + term_offset, node_map[source]);
        }
    }
}
}
}
}
