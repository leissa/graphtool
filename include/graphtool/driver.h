#pragma once

#include <fe/driver.h>
#include <fe/log.h>

#include "graphtool/tok.h"

namespace graphtool {

/// The keywords the Lexer looks up, keyed by the Sym it has just interned.
using Keys = fe::SymTab<Tok::Tag, Num_Keys>;

class Driver : public fe::Driver {
public:
    Driver();

    /// The keywords, interned once here and borrowed by every Lexer this Driver serves.
    const Keys& keys() const { return keys_; }

    /// What the analysis reports along the way - the chosen exit and the unreachable nodes; `main` configures it.
    fe::Log& log() { return log_; }
    const fe::Log& log() const { return log_; }

private:
    Keys keys_;
    fe::Log log_;
};

} // namespace graphtool
