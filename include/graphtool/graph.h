#pragma once

#include <fe/driver.h>

namespace graphtool {

using fe::Sym;

class Graph {
public:
    class Node {
    private:
        Node(Sym name)
            : name_(name) {}

    public:
        void link(Node* succ) {
            this->succs_.emplace(succ);
            succ->preds_.emplace(this);
        }
        const auto& preds() const { return preds_; }
        const auto& succs() const { return succs_; }

    private:
        Sym name_;
        std::unordered_set<Node*> preds_;
        std::unordered_set<Node*> succs_;

        friend class Graph;
    };

    Graph() = default;
    Graph(const Graph&) = delete;
    Graph(Graph&& other)
        : name_(other.name_)
        , nodes_(std::move(other.nodes_)) {}
    ~Graph();

    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) = delete;

    void set_name(Sym name) { name_ = name; }
    Node* node(Sym name);

    friend void swap(Graph& g1, Graph& g2) noexcept {
        using std::swap;
        swap(g1.name_, g2.name_);
        swap(g1.nodes_, g2.nodes_);
    }

private:
    Sym name_;
    fe::SymMap<Node*> nodes_;
};


} // namespace graphtool
