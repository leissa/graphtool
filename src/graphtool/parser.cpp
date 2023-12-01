#include "graphtool/parser.h"

#include <fstream>
#include <iostream>

using namespace std::literals;

namespace graphtool {

using Tag = Tok::Tag;

Parser::Parser(Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : graph_(driver)
    , lexer_(driver, istream, path) {
    init(path);
}

void Parser::err(const std::string& what, const Tok& tok, std::string_view ctxt) {
    driver().err(tok.loc(), "expected {}, got '{}' while parsing {}", what, tok, ctxt);
}

void Parser::syntax_err(Tag tag, std::string_view ctxt) {
    std::string msg("'");
    msg.append(Tok::str(tag)).append("'");
    err(msg, ctxt);
}

Graph Parser::parse_graph() {
    expect(Tag::K_digraph, "graph");
    if (auto tok = accept(Tok::Tag::V_sym)) graph_.set_name(tok.sym());
    parse_sub_graph("graph");
    expect(Tag::EoF, "graph");

    return std::move(graph_);
}

Graph::NodeSet Parser::parse_sub_graph(std::string_view ctxt) {
    Graph::NodeSet nodes;
    if (auto tok = accept(Tok::Tag::V_sym)) {
        nodes.emplace(graph_.node(tok.sym()));
    } else if (accept(Tag::D_brace_l)) {
        parse_stmt_list(nodes);
        expect(Tag::D_brace_r, "subgraph");
    } else {
        err("subgraph", ctxt);
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
