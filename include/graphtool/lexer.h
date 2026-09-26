#pragma once

#include <cassert>

#include <fe/lexer.h>

#include "graphtool/driver.h"
#include "graphtool/tok.h"

namespace graphtool {

class Lexer : public fe::Lexer<1, Lexer> {
public:
    Lexer(Driver&, const fe::Src&);

    Tok lex();                           ///< Get next Tok in stream.
    Driver& driver() { return driver_; } ///< fe::Lexer's default diagnostics go to its Driver::error.

private:
    void eat_comments();

    Driver& driver_;
    const Keys& keys_; ///< The Driver's keywords - see Driver::keys.
};

} // namespace graphtool
