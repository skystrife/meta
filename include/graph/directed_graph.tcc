/**
 * @file directed_graph.tcc
 * @author Sean Massung
 */

namespace meta
{
namespace graph
{
template <class Node, class Edge>
auto directed_graph<Node, Edge>::outgoing(node_id id) const -> const
    adjacency_list &
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return nodes_[id].second;
}

template <class Node, class Edge>
const std::vector<node_id>&
    directed_graph<Node, Edge>::incoming(node_id id) const
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return incoming_.at(id);
}

template <class Node, class Edge>
Node& directed_graph<Node, Edge>::node(node_id id)
{
    if (id >= size())
        throw directed_graph_exception{"node_id out of range"};

    return nodes_[id].first;
}

template <class Node, class Edge>
typename util::optional<Edge> directed_graph<Node, Edge>::edge(node_id source,
                                                               node_id dest)
{
    if (source >= size() || dest >= size())
        throw directed_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const std::pair<node_id, Edge>& p)
                           {
        return p.first == dest;
    });
    if (it != list.end())
        return {it->second};

    return {util::nullopt};
}

template <class Node, class Edge>
node_id directed_graph<Node, Edge>::insert(const Node& node)
{
    nodes_.emplace_back(node, adjacency_list{});
    incoming_.emplace_back(std::vector<node_id>{});
    return node_id{nodes_.size() - 1};
}

template <class Node, class Edge>
void directed_graph<Node, Edge>::add_edge(const Edge& edge, node_id source,
                                          node_id dest)
{
    if (source >= size() || dest >= size())
        throw directed_graph_exception{"node_id out of range"};

    auto& list = nodes_[source].second;
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const std::pair<node_id, Edge>& p)
                           {
        return p.first == dest;
    });
    if (it != list.end())
        throw directed_graph_exception{"attempted to add existing edge"};

    list.emplace_back(dest, edge);     // add outgoing edge from source to dest
    incoming_[source].push_back(dest); // add incoming edge to source
}

template <class Node, class Edge>
void directed_graph<Node, Edge>::add_edge(node_id source, node_id dest)
{
    add_edge(Edge{}, source, dest);
}

template <class Node, class Edge>
uint64_t directed_graph<Node, Edge>::size() const
{
    return nodes_.size();
}
}
}
