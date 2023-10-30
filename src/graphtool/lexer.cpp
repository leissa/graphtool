#include "graphtool/lexer.h"

#include <ranges>

#include <fe/loc.cpp.h>

using namespace std::literals;

namespace graphtool {

Lexer::Lexer(Driver& driver, std::istream& istream, const std::filesystem::path* path)
    : fe::Lexer<1, Lexer>(istream, path)
    , driver_(driver) {
    if (!istream_) throw std::runtime_error("stream is bad");
#define CODE(t, str) keywords_[driver_.sym(str)] = Tok::Tag::t;
    LET_KEY(CODE)
#undef CODE
}

Tok Lexer::lex() {
    while (true) {
        begin();

        if (accept(fe::utf8::EoF)) return {loc_, Tok::Tag::EoF};
        if (accept_if(isspace)) continue;
        if (accept(',')) return {loc_, Tok::Tag::T_comma};
        if (accept(';')) return {loc_, Tok::Tag::T_semicolon};
        if (accept('{')) return {loc_, Tok::Tag::D_brace_l};
        if (accept('}')) return {loc_, Tok::Tag::D_brace_r};

        if (accept('-')) {
            if (accept('>')) return {loc_, Tok::Tag::T_arrow};
            driver_.err({loc_.path, peek_}, "invalid token '-'; did you mean '->'?");
            continue;
        }

        // lex identifier or keyword
        if (accept_if<Append::Lower>([](int i) { return i == '_' || isalpha(i); })) {
            while (accept_if<Append::Lower>([](int i) { return i == '_' || isalpha(i) || isdigit(i); })) {}
            auto sym = driver_.sym(str_);
            if (auto i = keywords_.find(sym); i != keywords_.end()) return {loc_, i->second}; // keyword
            return {loc_, sym};                                                               // identifier
        }

        driver_.err({loc_.path, peek_}, "invalid input char: '{}'", (char)ahead());
        next();
    }
}

} // namespace graphtool
