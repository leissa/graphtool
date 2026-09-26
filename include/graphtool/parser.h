#pragma once

#include <fe/parser.h>

#include "graphtool/driver.h"
#include "graphtool/graph.h"
#include "graphtool/lexer.h"

namespace graphtool {

class Parser : public fe::Parser<Tok, Tok::Tag, 1, Parser> {
    using Super = fe::Parser<Tok, Tok::Tag, 1, Parser>;

public:
    Parser(Driver&, const fe::Src&);

    Driver& driver() { return lexer_.driver(); } ///< fe::Parser's default diagnostics go to its Driver::error.
    Lexer& lexer() { return lexer_; }

    Graph parse_graph();

private:
    Graph::NodeSet parse_sub_graph(fe::Cite ctxt);
    void parse_stmt_list(Graph::NodeSet&);
    void parse_edge_stmt(Graph::NodeSet&);

    Graph graph_;
    Lexer lexer_;

    friend Super;
};

} // namespace graphtool
