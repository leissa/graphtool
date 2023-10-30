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
    number_<0>();
    number_<1>();
}

template<size_t mode>
void Graph::number_() {
    auto [n, m] = entry_->number<mode>(0, 0);
    assert(n == m);
    rpo_[mode].resize(n);
    for (auto [_, node] : nodes()) {
        auto& order = node->order_[mode];
        if (order.post != Graph::Node::Not_Visited) {
            size_t i = n - order.post - 1;
            order.rp = i;
            rpo_[mode][i] = node;
        }
    }
}

template<size_t mode>
std::pair<size_t, size_t> Graph::Node::number(size_t pre, size_t post) {
    auto& order = order_[mode];
    if (order.pre == Not_Visited) {
        order.pre = pre++;
        for (auto succ : succs<mode>()) std::tie(pre, post) = succ->template number<mode>(pre, post);
        order.post = post++;
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
    return os << std::format("\t\"{}[{}|{}|{}][{}|{}|{}]\"", node.name(),
            node.pre<0>(), node.post<0>(), node.rp<0>(),
            node.pre<1>(), node.post<1>(), node.rp<1>());
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    os << std::format("digraph {} {{", graph.name()) << std::endl;
    for (const char* sep = ""; auto node : graph.rpo<0>()) {
        for (auto succ : node->succs())
            os << std::format("\t{} -> {}", *node, *succ) << sep;
        sep = "\n";
    }
    return os << '}' << std::endl;;
}

} // namespace graphtool
