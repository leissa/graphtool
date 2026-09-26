#include "graphtool/driver.h"

using namespace std::literals;

namespace graphtool {

Driver::Driver() {
#define CODE(t, str) keys_.emplace(sym(str##sv), Tok::Tag::t);
    GT_KEY(CODE)
#undef CODE
}

} // namespace graphtool
