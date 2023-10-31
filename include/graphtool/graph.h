#pragma once

#include <array>
#include <ostream>
#include <fe/driver.h>

namespace graphtool {

using fe::Sym;

class Graph {
public:
    class Node;
    using NodeSet = std::unordered_set<Node*>;

    class Node {
    private:
        static constexpr auto Not_Visited = size_t(-1);

        Node(Sym name)
            : name_(name) {}

    public:
        Sym name() const { return name_; }
        template<size_t mode> std::string dot() const;
        template<size_t mode> size_t pre()  const { return order_[mode].pre; }
        template<size_t mode> size_t post() const { return order_[mode].post; }
        template<size_t mode> size_t rp()   const { return order_[mode].rp; }
        template<size_t mode> Node* idom() const { return idom_[mode]; }
        template<size_t mode> Node*& idom() { return idom_[mode]; }
        template<size_t mode> const auto& children() const { return children_[mode]; }
        template<size_t mode> auto& children() { return children_[mode]; }

        void link(Node* succ) {
            this->succs_.emplace(succ);
            succ->preds_.emplace(this);
        }

        template<size_t mode> const auto& preds() const { return mode == 0 ? preds_ : succs_; }
        template<size_t mode> const auto& succs() const { return mode == 0 ? succs_ : preds_; }

    private:
        template<size_t> std::pair<size_t, size_t> number(size_t, size_t);

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

        friend class Graph;
    };

    Graph(const Graph&) = delete;
    Graph(fe::Driver& driver)
        : driver_(driver) {}
    Graph(Graph&& other)
        : driver_(other.driver_)
        , name_(other.name_)
        , entry_(other.entry_)
        , exit_(other.exit_)
        , nodes_(std::move(other.nodes_)) {}
    ~Graph();

    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) = delete;

    /// @name Getters
    ///@{
    fe::Driver& driver() { return driver_; }
    Sym name() const { return name_; }
    const auto& nodes() const { return nodes_; }
    template<size_t mode> Node* entry() const { return mode == 0 ? entry_ : exit_; }
    template<size_t mode> Node* exit() const { return mode == 0 ? exit_ : exit_; }
    template<size_t mode> const auto& rpo() const { return rpo_[mode]; }
    ///@}

    void set_name(Sym name) { name_ = name; }
    Node* node(Sym name);
    void critical_edge_elimination();
    void analyse();

    /// @name Output
    ///@{
    template<size_t> void dump_cfg(std::ostream&) const;
    template<size_t> void dump_dom_tree(std::ostream&) const;
    template<size_t> void dump_dom_frontiers(std::ostream&) const;
    ///@}

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
    template<size_t> void number();
    template<size_t> void dom();
    template<size_t> Node* lca(Node*, Node*);

    fe::Driver& driver_;
    Sym name_;
    Node* entry_ = nullptr;
    Node* exit_  = nullptr;
    fe::SymMap<Node*> nodes_;
    std::array<std::vector<Node*>, 2> rpo_;
};

} // namespace graphtool
