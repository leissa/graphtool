#include "graphtool/graph.h"

namespace graphtool {

Graph::~Graph() {
    for (auto [_, node] : nodes_) delete node;
}

Graph::Node* Graph::node(Sym name) {
    if (auto i = nodes_.find(name); i != nodes_.end()) return i->second;
    auto node = new Node(name);
    if (entry_ == nullptr) entry_ = node;
    exit_ = node;
    auto [_, ins] = nodes_.emplace(name, node);
    assert_unused(ins);
    return node;
}

/*
 * number
 */

void Graph::number() {
    auto [n, m] = entry_->number(0, 0);
    assert(n == m);
    rpo_.resize(n);
    for (auto [_, node] : nodes()) {
        if (node->post_ != Graph::Node::Not_Visited) {
            size_t i = n - node->post_ - 1;
            node->rp_ = i;
            rpo_[i] = node;
        }
    }
}

std::pair<size_t, size_t> Graph::Node::number(size_t pre, size_t post) {
    if (pre_ == Not_Visited) {
        pre_ = pre++;
        for (auto succ : succs()) std::tie(pre, post) = succ->number(pre, post);
        post_ = post++;
    }
    return {pre, post};
}

void Graph::critical_edge_elimination() {
    std::vector<std::pair<Node*, Node*>> crit;

    for (auto [_, node] : nodes_) {
        if (node->succs().size() > 1) {
            for (auto succ : node->succs()) {
                for (auto pred : succ->preds()) {
                    if (pred->succs().size() > 1) crit.emplace_back(succ, pred);
                }
            }
        }
    }

    for (auto [v, w] : crit) {
        auto name = driver().sym(v->name().str() + "." + w->name().str());
        auto x = node(name);
        v->succs_.erase(w);
        w->preds_.erase(v);
        v->link(x);
        x->link(w);
    }
}

/*
 * ostream
 */

std::ostream& operator<<(std::ostream& os, const Graph::Node& node) {
    return os << std::format("\t\"{}|{}|{}|{}\"", node.name(), node.pre(), node.post(), node.rp());
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    os << std::format("digraph {} {{", graph.name()) << std::endl;
    for (const char* sep = ""; auto node : graph.rpo()) {
        for (auto succ : node->succs())
            os << std::format("\t{} -> {}", *node, *succ) << sep;
        sep = "\n";
    }
    return os << '}' << std::endl;;
}

} // namespace graphtool
