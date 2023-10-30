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

void Graph::critical_edge_elimination() {
    std::vector<std::pair<Node*, Node*>> crit;
    auto x = exit_; // we create new nodes below - so memorize proper exit ...

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

    exit_ = x; // ... and restore again
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
    auto [n, m] = entry<mode>()->template number<mode>(0, 0);
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

/*
 * dom
 */

#if 0
template<size_t mode> void Graph::dom_() {
    // Cooper et al, 2001. A Simple, Fast Dominance Algorithm. http://www.cs.rice.edu/~keith/EMBED/dom.pdf
    idoms_[cfg().entry()] = cfg().entry();

    // all idoms different from entry are set to their first found dominating pred
    for (auto n : cfg().reverse_post_order().skip_front()) {
        for (auto pred : cfg().preds(n)) {
            if (cfg().index(pred) < cfg().index(n)) {
                idoms_[n] = pred;
                goto outer_loop;
            }
        }
        fe::unreachable();
outer_loop:;
    }

    for (bool todo = true; todo;) {
        todo = false;

        for (auto n : cfg().reverse_post_order().skip_front()) {
            const CFNode* new_idom = nullptr;
            for (auto pred : cfg().preds(n)) new_idom = new_idom ? least_common_ancestor(new_idom, pred) : pred;

            assert(new_idom);
            if (idom(n) != new_idom) {
                idoms_[n] = new_idom;
                todo      = true;
            }
        }
    }

    for (auto n : cfg().reverse_post_order().skip_front()) children_[idom(n)].push_back(n);
}
#endif

/*
 * ostream
 */

std::ostream& operator<<(std::ostream& os, const Graph::Node& node) {
    return os << std::format("\t\"{}\\n[{}|{}|{}]\\n[{}|{}|{}]\"", node.name(),
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
