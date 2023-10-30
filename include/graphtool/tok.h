#pragma once

#include <cassert>

#include <fe/format.h>
#include <fe/loc.h>
#include <fe/sym.h>

namespace graphtool {

using fe::Loc;
using fe::Pos;
using fe::Sym;

// clang-format off
#define LET_KEY(m)            \
    m(K_digraph,   "digraph") \

#define LET_VAL(m)                   \
    m(V_sym,        "<identifier>")  \

#define LET_TOK(m)                   \
    m(EoF,          "<end of file>") \
    /* delimiter */                  \
    m(D_brace_l,    "{")             \
    m(D_brace_r,    "}")             \
    /* further tokens */             \
    m(T_arrow,      "->")            \
    m(T_comma,      ",")             \
    m(T_semicolon,  ";")             \

#define CODE(t, str) + 1
constexpr auto Num_Keys = 0 LET_KEY(CODE);
#undef CODE

class Tok {
public:
    // clang-format off
    enum class Tag {
#define CODE(t, _) t,
        LET_KEY(CODE)
        LET_VAL(CODE)
        LET_TOK(CODE)
#undef CODE
    };
    // clang-format on

    enum class Prec {
        Error,
        Bottom,
        Add,
        Mul,
        Unary,
    };

    Tok() {}
    Tok(Loc loc, Tag tag)
        : loc_(loc)
        , tag_(tag) {}
    Tok(Loc loc, Sym sym)
        : loc_(loc)
        , tag_(Tag::V_sym)
        , sym_(sym) {}

    Loc loc() const { return loc_; }
    Tag tag() const { return tag_; }
    bool isa(Tag tag) const { return tag == tag_; }
    bool isa_key() const { return (int)tag() < Num_Keys; }

    Sym sym() const {
        assert(isa(Tag::V_sym));
        return sym_;
    }

    static std::string_view str(Tok::Tag);

    friend std::ostream& operator<<(std::ostream&, Tag);
    friend std::ostream& operator<<(std::ostream&, Tok);

private:
    Loc loc_;
    Tag tag_;
    Sym sym_;
};

} // namespace graphtool

template<>
struct std::formatter<graphtool::Tok> : fe::ostream_formatter {};
