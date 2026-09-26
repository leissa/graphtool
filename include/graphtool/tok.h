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
#define GT_KEY(m)             \
    m(K_digraph,   "digraph") \

#define GT_VAL(m)                    \
    m(V_sym,        "<identifier>")  \

#define GT_TOK(m)                    \
    m(EoF,          "<end of file>") \
    /* delimiter */                  \
    m(D_brace_l,    "{")             \
    m(D_brace_r,    "}")             \
    /* further tokens */             \
    m(T_arrow,      "->")            \
    m(T_comma,      ",")             \
    m(T_semicolon,  ";")             \

#define CODE(t, str) + 1
constexpr auto Num_Keys = 0 GT_KEY(CODE);
#undef CODE

class Tok {
public:
    // clang-format off
    enum class Tag {
        Nil,
#define CODE(t, _) t,
        GT_KEY(CODE)
        GT_VAL(CODE)
        GT_TOK(CODE)
#undef CODE
    };
    // clang-format on

    constexpr Tok() {}
    Tok(Loc loc, Tag tag)
        : loc_(loc)
        , tag_(tag) {}
    Tok(Loc loc, Sym sym)
        : loc_(loc)
        , tag_(Tag::V_sym)
        , sym_(sym) {}

    Loc loc() const { return loc_; }
    constexpr Tag tag() const { return tag_; }
    bool isa(Tag tag) const { return tag == tag_; }
    bool isa_key() const { return tag_ != Tag::Nil && (int)tag_ <= Num_Keys; }
    explicit operator bool() const { return tag_ != Tag::Nil; }

    Sym sym() const {
        assert(isa(Tag::V_sym));
        return sym_;
    }

    static std::string_view tag2str(Tok::Tag);

private:
    Loc loc_;
    Tag tag_ = Tag::Nil;
    Sym sym_;
};

std::ostream& operator<<(std::ostream&, Tok::Tag);
std::ostream& operator<<(std::ostream&, Tok);

} // namespace graphtool

// clang-format off
template<> struct std::formatter<graphtool::Tok>      : fe::ostream_formatter {};
template<> struct std::formatter<graphtool::Tok::Tag> : fe::ostream_formatter {};
// clang-format on
