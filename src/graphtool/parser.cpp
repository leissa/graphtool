#include "graphtool/parser.h"

#include <fstream>
#include <iostream>

using namespace std::literals;

namespace graphtool {

using Tag = Tok::Tag;

Parser::Parser(Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : lexer_(driver, istream, path)
    , error_(driver.sym("<error>"s)) {
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

Sym Parser::parse_sym(std::string_view ctxt) {
    if (ahead().isa(Tag::V_sym)) return lex().sym();
    err("identifier", ctxt);
    return error_;
}

Graph::Node* Parser::parse_node() {
    auto name = parse_sym("name of a node");
    return graph_.node(name);
}

Graph Parser::parse_graph() {
    expect(Tag::K_digraph, "graph");
    graph_.set_name(parse_sym("graph name"));
    expect(Tag::D_brace_l, "graph");

    while (ahead().isa(Tag::V_sym)) parse_path();

    expect(Tag::D_brace_r, "graph");
    expect(Tag::EoF, "graph");

    return std::move(graph_);
}

void Parser::parse_path() {
    auto lhs = parse_node();

    while (accept(Tag::T_arrow)) {
        auto rhs = parse_node();
        lhs->link(rhs);
        lhs = rhs;
    }

    expect(Tag::T_semicolon, "path");
}

} // namespace graphtool
