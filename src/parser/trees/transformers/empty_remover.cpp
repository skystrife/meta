/**
 * @file empty_remover.cpp
 * @author Chase Geigle
 */

#include "parser/trees/transformers/empty_remover.h"
#include "util/shim.h"

namespace meta
{
namespace parser
{

std::unique_ptr<node> empty_remover::transform(const leaf_node& lnode)
{
    const static class_label none{"-NONE-"};
    if (lnode.category() == none)
        return nullptr;
    return make_unique<leaf_node>(lnode);
}

std::unique_ptr<node> empty_remover::transform(const internal_node& inode)
{
    std::vector<std::unique_ptr<node>> children;
    inode.each_child([&](const node* child)
            {
                auto trns_child = child->accept(*this);
                if (trns_child)
                    children.emplace_back(std::move(trns_child));
            });
    if (children.empty())
        return nullptr;
    return make_unique<internal_node>(inode.category(), std::move(children));
}

}
}
