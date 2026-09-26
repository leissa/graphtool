#include "graphtool/parser.h"

namespace graphtool {

using Tag = Tok::Tag;

Parser::Parser(Driver& driver, const fe::Src& src)
    : graph_(driver)
    , lexer_(driver, src) {
    init();
}

Graph Parser::parse_graph() {
    expect(Tag::K_digraph, "graph");
    if (auto tok = accept(Tag::V_sym)) graph_.set_name(tok.sym());
    parse_sub_graph("graph");
    expect(Tag::EoF, "graph");

    return std::move(graph_);
}

Graph::NodeSet Parser::parse_sub_graph(fe::Cite ctxt) {
    recover(Tag::D_brace_r, ctxt);
    Graph::NodeSet nodes;

    if (auto tok = accept(Tag::V_sym)) {
        nodes.emplace(graph_.node(tok.sym()));
    } else if (auto brace_l = accept(Tag::D_brace_l)) {
        auto _ = anchor(brace_l, Tag::D_brace_r);
        parse_stmt_list(nodes);
        expect(Tag::D_brace_r, "subgraph");
    } else {
        syntax_err("subgraph", ctxt);
    }

    return nodes;
}

void Parser::parse_stmt_list(Graph::NodeSet& nodes) {
    while (true) {
        // clang-format off
        switch (ahead().tag()) {
            case Tag::T_comma:
            case Tag::T_semicolon:  lex(); continue;
            case Tag::D_brace_l:
            case Tag::V_sym:        parse_edge_stmt(nodes); continue;
            default:                return;
        }
        // clang-format on
    }
}

void Parser::parse_edge_stmt(Graph::NodeSet& nodes) {
    auto lhs = parse_sub_graph("edge statement");
    nodes.insert(lhs.begin(), lhs.end());
    while (accept(Tag::T_arrow)) {
        auto rhs = parse_sub_graph("edge statement");
        nodes.insert(rhs.begin(), rhs.end());

        for (auto pred : lhs) {
            for (auto succ : rhs) pred->link(succ);
        }

        lhs = std::move(rhs);
    }
}

} // namespace graphtool
