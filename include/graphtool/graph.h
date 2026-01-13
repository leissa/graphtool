#pragma once

#include <array>
#include <ostream>
#include <vector>

#include <fe/driver.h>

namespace graphtool {

static constexpr auto Not_Visited = size_t(-1);

using fe::Sym;

class Graph {
public:
    class Node;
    using NodeSet = std::unordered_set<Node*>;

    class Node {
    private:
        Node(Sym name)
            : name_(name) {}

    public:
        Sym name() const { return name_; }

        void link(Node* succ) {
            this->succs_.emplace(succ);
            succ->preds_.emplace(this);
        }

    private:
        Sym name_;
        NodeSet preds_, succs_;

        struct Order {
            size_t pre  = Not_Visited;
            size_t post = Not_Visited;
            size_t rp   = Not_Visited;
        };

        std::array<Order, 2> order_;
        std::array<Node*, 2> idom_;
        std::array<std::vector<Node*>, 2> children_;
        std::array<NodeSet, 2> frontier_;

        friend class Graph;
        template<size_t M>
        friend class BiGraph;
    };

    Graph(const Graph&) = delete;
    Graph(fe::Driver& driver)
        : driver_(driver) {}
    Graph(Graph&& other) noexcept
        : driver_(other.driver_)
        , name_(other.name_)
        , entry_(other.entry_)
        , exit_(other.exit_)
        , nodes_(std::move(other.nodes_)) {}
    ~Graph();

    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&)      = delete;

    /// @name Getters
    ///@{
    fe::Driver& driver() { return driver_; }
    Sym name() const { return name_; }
    const auto& nodes() const { return nodes_; }
    ///@}

    void set_name(Sym name) { name_ = name; }
    Node* node(Sym name); ///< Construct Graph::Node without duplicates.
    void critical_edge_elimination();

    friend void swap(Graph& g1, Graph& g2) noexcept {
        using std::swap;
        // clang-format off
        swap(g1.driver_, g2.driver_);
        swap(g1.name_,   g2.name_);
        swap(g1.entry_,  g2.entry_);
        swap(g1.exit_,   g2.exit_);
        swap(g1.nodes_,  g2.nodes_);
        // clang-format on
    }

private:
    fe::Driver& driver_;
    Sym name_;
    Node* entry_ = nullptr;
    Node* exit_  = nullptr;
    fe::SymMap<Node*> nodes_;
    std::array<std::vector<Node*>, 2> rpo_;

    template<size_t M>
    friend class BiGraph;
};

template<size_t M>
class BiGraph {
public:
    using Node = Graph::Node;

    BiGraph(Graph& graph)
        : graph_(graph) {
        number();
        dom();
        dom_frontiers();
    }

    /// @name Node Wrappers
    ///@{
    static std::string dot(Node*);
    static auto& order(Node* n) { return n->order_[M]; }
    static size_t pre(Node* n) { return order(n).pre; }
    static size_t post(Node* n) { return order(n).post; }
    static size_t rp(Node* n) { return order(n).rp; }
    static Node*& idom(Node* n) { return n->idom_[M]; }
    static auto& children(Node* n) { return n->children_[M]; }
    static auto& frontier(Node* n) { return n->frontier_[M]; }
    static const auto& preds(Node* n) { return M == 0 ? n->preds_ : n->succs_; }
    static const auto& succs(Node* n) { return M == 0 ? n->succs_ : n->preds_; }
    static std::pair<size_t, size_t> number(Node*, size_t, size_t);
    ///@}

    /// @name Getters
    ///@{
    Sym name() const { return graph_.name(); }
    Node* entry() const { return M == 0 ? graph_.entry_ : graph_.exit_; }
    Node* exit() const { return M == 0 ? graph_.exit_ : graph_.entry_; }
    auto& rpo() { return graph_.rpo_[M]; }
    const auto& rpo() const { return graph_.rpo_[M]; }
    ///@}

    /// @name Output
    ///@{
    void dump_cfg(std::ostream&) const;
    void dump_dom_tree(std::ostream&) const;
    void dump_dom_frontiers(std::ostream&) const;
    ///@}

private:
    void number();
    void dom();
    void dom_frontiers();
    Node* lca(Node*, Node*);

    Graph& graph_;
};

} // namespace graphtool
