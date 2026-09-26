#include "graphtool/tok.h"

#include <fe/assert.h>

using namespace std::literals;

namespace graphtool {

std::string_view Tok::tag2str(Tok::Tag tag) {
    switch (tag) {
#define CODE(t, str) \
    case Tok::Tag::t: return str##sv;
        GT_KEY(CODE)
        GT_VAL(CODE)
        GT_TOK(CODE)
#undef CODE
        default: fe::unreachable();
    }
}

std::ostream& operator<<(std::ostream& o, Tok::Tag tag) { return o << Tok::tag2str(tag); }

std::ostream& operator<<(std::ostream& o, Tok tok) {
    if (tok.isa(Tok::Tag::V_sym)) return o << *tok.sym();
    return o << Tok::tag2str(tok.tag());
}

} // namespace graphtool
