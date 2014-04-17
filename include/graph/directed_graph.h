/**
 * @file graph.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DIRECTED_GRAPH_H_
#define META_DIRECTED_GRAPH_H_

#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "meta.h"
#include "util/optional.h"
#include "graph/default_node.h"
#include "graph/default_edge.h"

namespace meta
{
namespace graph
{
/**
 * A (currently) simple class to represent a directed graph in memory.
 */
template <class Node = default_node, class Edge = default_edge>
class directed_graph
{
  public:
    using AdjacencyList = std::unordered_map<node_id, Edge>;

    /**
     * Basic exception for directed_graph interactions.
     */
    class directed_graph_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    /**
     * Constructor to create an empty directed_graph.
     */
    // directed_graph();

    /**
     * @param id
     * @return the adjacent edges and node_ids to the given node
     */
    const AdjacencyList& adjacent(node_id id) const;

    /**
     * @param id
     * @return the Node object that the id represents
     */
    Node& node(node_id id);

    /**
     * @param source
     * @param dest
     * @return an optional edge connecting source and dest
     */
    util::optional<Edge> edge(node_id source, node_id dest);

    /**
     * @param node The new object to add into the graph
     * @return the id of the inserted node
     */
    node_id insert(const Node& node);

    /**
     * @param source
     * @param dest
     */
    void add_edge(const Edge& edge, node_id source, node_id dest);

    /**
     * @return the size of this graph (number of nodes), which is the
     * range for a valid node_id
     */
    uint64_t size() const;

  private:
    /// Each Node object is indexed by its id.
    std::vector<std::pair<Node, AdjacencyList>> nodes_;
};
}
}

#include "graph/directed_graph.tcc"
#endif
