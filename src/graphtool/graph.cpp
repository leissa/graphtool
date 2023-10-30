#include "graphtool/graph.h"

namespace graphtool {

Graph::~Graph() {
    for (auto [_, node] : nodes_) delete node;
}

Graph::Node* Graph::node(Sym name) {
    if (auto i = nodes_.find(name); i != nodes_.end()) return i->second;
    auto node = new Node(name);
    auto [_, ins] = nodes_.emplace(name, node);
    assert_unused(ins);
    return node;
}

} // namespace graphtool
