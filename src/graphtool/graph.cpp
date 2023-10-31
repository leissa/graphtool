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
    exit_         = node;
    auto [_, ins] = nodes_.emplace(name, node);
    assert_unused(ins);
    return node;
}

void Graph::critical_edge_elimination() {
    std::vector<std::pair<Node*, Node*>> crit;
    auto x = exit_; // we create new nodes below - so memorize proper exit ...

    for (auto [_, node] : nodes_) {
        if (node->succs_.size() > 1) {
            for (auto succ : node->succs_) {
                if (succ->preds_.size() > 1) crit.emplace_back(node, succ);
            }
        }
    }

    for (auto [v, w] : crit) {
        auto name = driver().sym(v->name().str() + "." + w->name().str());
        auto x    = node(name);
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

template<size_t M>
void BiGraph<M>::number() {
    auto [n, m] = number(entry(), 0, 0);
    assert(n == m);
    rpo().resize(n);
    for (auto [_, node] : graph_.nodes()) {
        auto& order = this->order(node);
        if (order.post != Not_Visited) {
            size_t i = n - order.post - 1;
            order.rp = i;
            rpo()[i] = node;
        }
    }
}

template<size_t M>
std::pair<size_t, size_t> BiGraph<M>::number(Node* n, size_t pre, size_t post) {
    auto& order = BiGraph<M>::order(n);
    if (order.pre == Not_Visited) {
        order.pre = pre++;
        for (auto succ : succs(n)) std::tie(pre, post) = number(succ, pre, post);
        order.post = post++;
    }
    return {pre, post};
}

/*
 * dom
 */

// Cooper et al, 2001. A Simple, Fast Dominance Algorithm. http://www.cs.rice.edu/~keith/EMBED/dom.pdf
template<size_t M>
void BiGraph<M>::dom() {
    idom(entry()) = entry();

    // all idoms different from entry are set to their first found dominating pred
    for (auto n : rpo() | std::views::drop(1)) {
        for (auto pred : preds(n)) {
            if (rp(pred) < rp(n)) {
                idom(n) = pred;
                break;
            }
        }
    }

    for (bool todo = true; todo;) {
        todo = false;

        for (auto n : rpo() | std::views::drop(1)) {
            Node* new_idom = nullptr;
            for (auto pred : preds(n)) new_idom = new_idom ? lca(new_idom, pred) : pred;

            assert(new_idom);
            if (idom(n) != new_idom) {
                idom(n) = new_idom;
                todo    = true;
            }
        }
    }

    for (auto n : rpo() | std::views::drop(1)) children(idom(n)).emplace_back(n);
}

template<size_t M>
Graph::Node* BiGraph<M>::lca(Node* i, Node* j) {
    assert(i && j);
    while (rp(i) != rp(j)) {
        while (rp(i) < rp(j)) j = idom(j);
        while (rp(j) < rp(i)) i = idom(i);
    }
    return i;
}

template<size_t M>
void BiGraph<M>::dom_frontiers() {
    for (auto n : rpo() | std::views::drop(1)) {
        const auto& preds = this->preds(n);
        if (preds.size() > 1) {
            auto idom = this->idom(n);
            for (auto pred : preds) {
                for (auto i = pred; i != idom; i = this->idom(i)) frontier(i).emplace(n);
            }
        }
    }
}

/*
 * output
 */

template<size_t M>
std::string BiGraph<M>::dot(Node* n) {
    return std::format("\"{}\\n[{}|{}|{}]\"", n->name(), pre(n), post(n), rp(n));
}

template<size_t M>
void BiGraph<M>::dump_cfg(std::ostream& os) const {
    os << std::format("digraph {} {{", name()) << std::endl;
    for (const char* sep = ""; auto n : rpo()) {
        for (auto succ : succs(n)) {
            os << sep << std::format("\t{} -> {}", dot(n), dot(succ));
            sep = "\n";
        }
    }
    os << std::endl << '}' << std::endl;
}

template<size_t M>
void BiGraph<M>::dump_dom_tree(std::ostream& os) const {
    os << std::format("digraph {} {{", name()) << std::endl;
    for (const char* sep = ""; auto n : rpo()) {
        for (auto child : children(n)) {
            os << sep << std::format("\t{} -> {}", dot(n), dot(child));
            sep = "\n";
        }
    }
    os << std::endl << '}' << std::endl;
}

template<size_t M>
void BiGraph<M>::dump_dom_frontiers(std::ostream& os) const {
    os << std::format("digraph {} {{", name()) << std::endl;
    os << "\trankdir=\"BT\"" << std::endl;
    for (const char* sep = ""; auto n : rpo()) {
        for (auto fron : frontier(n)) {
            os << sep << std::format("\t{} -> {}", dot(n), dot(fron));
            sep = "\n";
        }
    }
    os << std::endl << '}' << std::endl;
}

// instantiate templates

template class BiGraph<0>;
template class BiGraph<1>;

} // namespace graphtool
