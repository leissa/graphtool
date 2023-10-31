#include "graphtool/graph.h"

#include <ranges>

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
        if (node->succs<0>().size() > 1) {
            for (auto succ : node->succs<0>()) {
                if (succ->preds<0>().size() > 1) crit.emplace_back(node, succ);
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

void Graph::analyse() {
    number<0>();
    number<1>();
    dom<0>();
    dom<1>();
    dom_frontiers<0>();
    dom_frontiers<1>();
}

/*
 * number
 */

template<size_t mode>
void Graph::number() {
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

// Cooper et al, 2001. A Simple, Fast Dominance Algorithm. http://www.cs.rice.edu/~keith/EMBED/dom.pdf
template<size_t mode> void Graph::dom() {
    entry<mode>()->template idom<mode>() = entry<mode>();

    // all idoms different from entry are set to their first found dominating pred
    for (auto node : rpo<mode>() | std::views::drop(1)) {
        for (auto pred : node->template preds<mode>()) {
            if (pred->template rp<mode>() < node->template rp<mode>()) {
                node->template idom<mode>() = pred;
                break;
            }
        }
    }

    for (bool todo = true; todo;) {
        todo = false;

        for (auto node : rpo<mode>() | std::views::drop(1)) {
            Node* new_idom = nullptr;
            for (auto pred : node->template preds<mode>()) new_idom = new_idom ? lca<mode>(new_idom, pred) : pred;

            assert(new_idom);
            if (node->template idom<mode>() != new_idom) {
                node->template idom<mode>() = new_idom;
                todo = true;
            }
        }
    }

    for (auto node : rpo<mode>() | std::views::drop(1))
        node->template idom<mode>()->template children<mode>().emplace_back(node);
}

template<size_t mode>
Graph::Node* Graph::lca(Node* i, Node* j) {
    assert(i && j);
    while (i->template rp<mode>() != j->template rp<mode>()) {
        while (i->template rp<mode>() < j->template rp<mode>()) j = j->template idom<mode>();
        while (j->template rp<mode>() < i->template rp<mode>()) i = i->template idom<mode>();
    }
    return i;
}

template<size_t mode>
void Graph::dom_frontiers() {
    for (auto node : rpo<mode>() | std::views::drop(1)) {
        const auto& preds = node->template preds<mode>();
        if (preds.size() > 1) {
            auto idom = node->template idom<mode>();
            for (auto pred : preds) {
                for (auto i = pred; i != idom; i = i->template idom<mode>()) i->template frontier<mode>().emplace(node);
            }
        }
    }
}

/*
 * output
 */

template<size_t mode>
std::string Graph::Node::dot() const {
    return std::format("\"{}\\n[{}|{}|{}]\"", name(), pre<mode>(), post<mode>(), rp<mode>());
}

template<size_t mode>
void Graph::dump_cfg(std::ostream& os) const {
    os << std::format("digraph {} {{", name()) << std::endl;
    for (const char* sep = ""; auto node : rpo<mode>()) {
        for (auto succ : node->template succs<mode>()) {
            os << sep << std::format("\t{} -> {}", node->template dot<mode>(), succ->template dot<mode>());
            sep = "\n";
        }
    }
    os << std::endl << '}' << std::endl;
}

template<size_t mode>
void Graph::dump_dom_tree(std::ostream& os) const {
    os << std::format("digraph {} {{", name()) << std::endl;
    for (const char* sep = ""; auto node : rpo<mode>()) {
        for (auto child : node->template children<mode>()) {
            os << sep << std::format("\t{} -> {}", node->template dot<mode>(), child->template dot<mode>());
            sep = "\n";
        }
    }
    os << std::endl << '}' << std::endl;
}

template<size_t mode>
void Graph::dump_dom_frontiers(std::ostream& os) const {
    os << std::format("digraph {} {{", name()) << std::endl;
    os << "\trankdir=\"BT\"" << std::endl;
    for (const char* sep = ""; auto node : rpo<mode>()) {
        for (auto fron : node->template frontier<mode>()) {
            os << sep << std::format("\t{} -> {}", node->template dot<mode>(), fron->template dot<mode>());
            sep = "\n";
        }
    }
    os << std::endl << '}' << std::endl;
}

// instantiate templates

template void Graph::dump_cfg<0>(std::ostream&) const;
template void Graph::dump_cfg<1>(std::ostream&) const;
template void Graph::dump_dom_tree<0>(std::ostream&) const;
template void Graph::dump_dom_tree<1>(std::ostream&) const;
template void Graph::dump_dom_frontiers<0>(std::ostream&) const;
template void Graph::dump_dom_frontiers<1>(std::ostream&) const;

} // namespace graphtool
