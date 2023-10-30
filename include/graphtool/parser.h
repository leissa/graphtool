#pragma once

#include <fe/parser.h>

#include "graphtool/driver.h"
#include "graphtool/graph.h"
#include "graphtool/lexer.h"

namespace graphtool {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
public:
    Parser(Driver&, std::istream&, const std::filesystem::path* = nullptr);

    Driver& driver() { return lexer_.driver(); }
    Lexer& lexer() { return lexer_; }

    Graph parse_graph();

private:
    Sym parse_sym(std::string_view ctxt = {});
    Graph::Node* parse_node();
    void parse_path();

    /// Issue an error message of the form:
    /// `expected <what>, got '<tok>' while parsing <ctxt>`
    void err(const std::string& what, const Tok& tok, std::string_view ctxt);

    /// Same above but uses Parser::ahead() as Tok%en.
    void err(const std::string& what, std::string_view ctxt) { err(what, ahead(), ctxt); }

    void syntax_err(Tok::Tag tag, std::string_view ctxt);

    Graph graph_;
    Lexer lexer_;
    Sym error_;

    friend class fe::Parser<Tok, Tok::Tag, 1, Parser>;
};

} // namespace graphtool
