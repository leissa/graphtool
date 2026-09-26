#pragma once

#include <fe/driver.h>

#include "graphtool/tok.h"

namespace graphtool {

/// The keywords the Lexer looks up, keyed by the Sym it has just interned.
using Keys = fe::SymTab<Tok::Tag, Num_Keys>;

class Driver : public fe::Driver {
public:
    Driver();

    /// The keywords, interned once here and borrowed by every Lexer this Driver serves.
    const Keys& keys() const { return keys_; }

private:
    Keys keys_;
};

} // namespace graphtool
