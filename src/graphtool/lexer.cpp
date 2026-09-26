#include "graphtool/lexer.h"

namespace graphtool {

namespace utf8 = fe::utf8;

namespace {
bool is_id_start(char32_t c) { return c == '_' || utf8::isalpha(c); }
bool is_id_cont(char32_t c) { return c == '_' || utf8::isalnum(c); }
} // namespace

Lexer::Lexer(Driver& driver, const fe::Src& src)
    : fe::Lexer<1, Lexer>(src)
    , driver_(driver)
    , keys_(driver.keys()) {}

Tok Lexer::lex() {
    while (true) {
        start();

        if (accept(utf8::EoF)) return {loc_, Tok::Tag::EoF};
        if (accept(utf8::isspace)) continue;
        if (recover_utf8()) continue;
        if (accept('{')) return {loc_, Tok::Tag::D_brace_l};
        if (accept('}')) return {loc_, Tok::Tag::D_brace_r};
        if (accept(',')) return {loc_, Tok::Tag::T_comma};
        if (accept(';')) return {loc_, Tok::Tag::T_semicolon};

        if (accept('-')) {
            if (accept('>')) return {loc_, Tok::Tag::T_arrow};
            error().e(loc_, "invalid token `-`").n("did you mean `->`?");
            continue;
        }

        if (accept('/')) {
            if (accept('*')) { // C-style comment
                eat_comments();
                continue;
            }
            if (accept('/')) { // C++-style comment
                accept_while_none_of('\n');
                continue;
            }
            error().e(loc_, "invalid token `/`").n("did you mean `/*` or `//`?");
            continue;
        }

        // lex identifier or keyword
        if (accept(is_id_start)) {
            accept_while(is_id_cont);
            auto sym = driver_.sym(view());
            if (auto tag = keys_.find(sym)) return {loc_, *tag}; // keyword
            return {loc_, sym};                                  // identifier
        }

        recover_char();
    }
}

void Lexer::eat_comments() {
    accept_until("*/");
    if (!accept('*')) {
        error().e(loc_, "non-terminated multiline comment");
        return;
    }
    accept('/');
}

} // namespace graphtool
