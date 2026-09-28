# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

GraphTool reads a subset of [Graphviz'](https://graphviz.org) DOT language and computes dominance-related properties
(pre/post/reverse-post order, dominance and postdominance trees, and their frontiers), writing each result as a `.dot`
file next to the input. Like [let](https://github.com/leissa/let), it showcases the [FE](https://leissa.github.io/fe/)
compiler-frontend library, which lives in `submodules/fe` and is maintained by the same author. Changes here often go
hand in hand with changes in the fe submodule.

## Build

```sh
git clone --recurse-submodules <url>   # fe submodule is required
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```

Requires CMake 3.29 and C++23 (CI builds with gcc-15 on Linux, Apple clang on macOS, MSVC on Windows). The binary lands
in `build/bin/graphtool`; run it as `./build/bin/graphtool test/cytron.dot` (`-c` eliminates critical edges first, and
the `--loc-style`/`--no-snippet`/`--gutter`/`--max-rows`/`--max-errors`/`--werror` switches tune the diagnostics).

## Tests

`test/run.sh` is the whole suite and the only thing CI runs: every input in `test/` becomes two golden-file cases,
`<name>` and `<name>-c` (the same input with `-c`), each of which copies its input into `build/test/<case>/` - the
build tree rather than the source tree, and `--out` moves it elsewhere - runs GraphTool there and diffs all six
results against the flat golden files `test/golden/<case>.<suffix>.dot`. `--bless` regenerates those golden files -
read the diff before committing it, because a bless turns a regression into the new expectation. The skip list lives
in the script's `skip`, not in the workflows, so every runner skips alike; it is empty at the moment.

`CMakeLists.txt` asks `test/run.sh --list` at configure time and registers one CTest per case, so `make test` (or
`ctest --test-dir build --output-on-failure`) runs the same script; `-DBUILD_TESTING=OFF` or a missing `bash` drops
the tests. Because that list is baked in at configure time, a new input in `test/` re-runs CMake via a
`CONFIGURE_DEPENDS` glob. CI builds every platform in Debug and Release and runs `ctest`, on Linux additionally
`test/run.sh --valgrind` and a build under ASan+LSan+UBSan.

## Architecture

Classic pipeline, one class per stage, all deriving from FE's CRTP base classes:

- **`include/graphtool/tok.h`** - `Tok` built from the X-macros `GT_KEY`, `GT_VAL`, `GT_TOK`; tags and their string
  forms both derive from them, so adding a token means extending one macro. `Tok::tag2str` is the name FE's
  `Parser::tag2str_` looks for - a bare enumerator would otherwise render as its number.
- **`include/graphtool/driver.h`** + **`src/graphtool/driver.cpp`** - `graphtool::Driver : fe::Driver` inherits the
  `SymPool`, the `SrcMap`, the `Diag` and the `Error` from its base and adds the interned keywords. `Driver::keys()` is
  an `fe::SymTab<Tok::Tag, Num_Keys>` - a fixed-capacity `Sym`-keyed table built once in the constructor and borrowed
  by the `Lexer`; intern with `Driver::sym` first, then look up, as `SymTab` hashes the interned pointer and has no
  heterogeneous lookup. `Driver::log()` is an `fe::Log` - `main.cpp` points it at `std::cerr` and raises its level from
  `Error` by one per `-V` - through which `BiGraph::number` reports the chosen exit and every node the entry/exit does
  not reach, at the `Loc` of the node's first mention (the exit's is its last one, `Graph::exit_loc_`).
- **Diagnostics** live in the driver: every building block reports into `driver().error()` via `error().e(loc, ...)`,
  and `error().n(...)` adds a note. `fe::Error` puts an `fe::Snippet` source excerpt under every message and renders
  `` `code` `` citations in color, so phrase messages with backticks, not quotes.
- **`src/graphtool/lexer.cpp`** - `Lexer : fe::Lexer<1, Lexer>`. The whole source sits in the lexer's buffer, so `loc_`
  *is* the token and `view()` returns its slice for free. `recover_utf8` runs before the token dispatch and
  `recover_char` closes it; comments are skipped with `accept_while_none_of`/`accept_until`, which never decode a code
  point.
- **`src/graphtool/parser.cpp`** - `Parser : fe::Parser<Tok, Tok::Tag, 1, Parser>`, recursive descent. Parse errors do
  not throw; they accumulate in the `Driver`'s `fe::Error`, and `main.cpp` calls `driver.error().ack()` afterwards,
  which throws an `fe::Error::Bail` if anything was collected - so the analysis only ever sees a well-formed graph.
  `ack` must be called while the `Driver` is still alive, because every `Loc` in the `Error` points into its `SrcMap`.
  A subgraph anchors its `}` (`fe::Parser::ScopedAnchor`) so a stray one is discarded by `recover` rather than
  swallowed; the anchor is handed the `{` that opened the subgraph, which is what FE's `syntax_err` notes when the
  `}` never comes - GraphTool writes no diagnostic of its own for it.
- **`include/graphtool/graph.h`** + **`src/graphtool/graph.cpp`** - `Graph` owns the nodes; `BiGraph<M>` is the same
  graph read forwards (`M == 0`) or backwards (`M == 1`), which is what makes dominance and postdominance one
  algorithm. Every per-direction datum on a `Node` is a two-element array indexed by `M`. `BiGraph` is explicitly
  instantiated for both at the bottom of `graph.cpp`.

Containers follow FE: `ankerl::unordered_dense` (directly, or through `fe::SymMap`) instead of `std::unordered_*`, and
`fe::Vector` instead of `std::vector`. That is also what makes the output reproducible - a dense set iterates in
insertion order, where the `std::unordered_set<Node*>` it replaced iterated in pointer-hash order.

`main.cpp` declares its switches with `fe::Cli` (`submodules/fe/include/fe/cli.h`); `cli.help`/`operator<<` render the
`--help` text, which `README.md`'s usage block is a copy of - keep both in sync when adding an option. The grammar is
also documented in `README.md` - update it when changing the language.

## Formatting & versioning

- clang-format is enforced via pre-commit (`.pre-commit-config.yaml`); `.clang-format` is at the repo root. Code uses
  `// clang-format off/on` around the X-macro tables.
- The `project(... VERSION)` lines are the only place a version number lives: `CMakeLists.txt` turns them into the
  `GRAPHTOOL_VERSION`/`FE_VERSION` defines that `--version` prints (fe's `fe_VERSION` is scoped to its own directory,
  hence the `get_directory_property`), so a bump needs no other edit.
