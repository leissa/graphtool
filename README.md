# GraphTool

[![linux](https://img.shields.io/github/actions/workflow/status/leissa/graphtool/linux.yml?logo=linux&logoColor=white&label=linux&link=https%3A%2F%2Fgithub.com%2Fleissa%2Fgraphtool%2Factions%2Fworkflows%2Flinux.yml)](https://github.com/leissa/graphtool/actions/workflows/linux.yml)
[![macos](https://img.shields.io/github/actions/workflow/status/leissa/graphtool/macos.yml?logo=apple&logoColor=white&label=macos&link=https%3A%2F%2Fgithub.com%2Fleissa%2Fgraphtool%2Factions%2Fworkflows%2Fmacos.yml)](https://github.com/leissa/graphtool/actions/workflows/macos.yml)
[![windows](https://img.shields.io/github/actions/workflow/status/leissa/graphtool/windows.yml?logo=windows&logoColor=white&label=windows&link=https%3A%2F%2Fgithub.com%2Fgraphtool%2Fleissa%2Factions%2Fworkflows%2Fwindows.yml)](https://github.com/leissa/graphtool/actions/workflows/windows.yml)

A small tool that reads a subset from [Graphviz'](https://graphviz.org) [DOT language](https://graphviz.org/doc/info/lang.html) and calculates several [dominance-related](https://en.wikipedia.org/wiki/Dominator_(graph_theory)) properties:
* pre-, post-, and reverse-post order numbers for the input rooted at the [entry and exit](#entry--exit)
* dominance tree
* postdominance tree
* dominance frontiers
* postdominance tree frontieres (aka control dependence)
* optionally eliminate [critical edges](https://en.wikipedia.org/wiki/Control-flow_graph#Special_edges) beforehand

## Usage

```
Usage: graphtool [options] <file>

Computes dominance-related properties of a Graphviz DOT digraph.

Arguments:
  <file>                   Input file.

Options:
  -h, --help               Display this help and exit.
  -v, --version            Display version info and exit.
  -c, --crit               Eliminate critical edges.

Diagnostics:
      --loc-style <style>  How a diagnostic spells out a source location: `full`
                           (`path:row:col-row:col`), `rowcol` (`path:row:col`),
                           `row` (`path:row`), or msvc (`path(row,col)`).
      --no-snippet         Does not render the offending source line and caret
                           underneath a diagnostic.
      --gutter <width>     Width of a diagnostic's line-number column. [default:
                           `5`]
      --max-rows <num>     Maximum number of rows a diagnostic's snippet renders
                           before eliding its middle; `0` elides nothing.
                           [default: `8`]
      --max-errors <num>   Maximum number of errors to report before dropping
                           the rest; `0` reports all of them. [default: `0`]
      --werror             Treats warnings as errors.

The results are written next to `<file>` as `<file>.forward.dot`, `<file>.backward.dot`, `<file>.dom_tree.dot`, `<file>.postdom_tree.dot`, `<file>.dom_frontiers.dot`, and `<file>.postdom_frontiers.dot`.
```

## Building

If you have a [GitHub account setup with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh), just do this:
```sh
git clone --recurse-submodules git@github.com:leissa/graphtool.git
```
Otherwise, clone via HTTPS:
```sh
git clone --recurse-submodules https://github.com/leissa/graphtool.git
```
Then, build with:
```sh
cd graphtool
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```
For a `Release` build simply use `-DCMAKE_BUILD_TYPE=Release`.
GraphTool needs CMake 3.29 and a C++23 compiler; it builds upon [FE](https://leissa.github.io/fe/), which lives in `submodules/fe`.

Invoke the GraphTool like so:
```sh
./build/bin/graphtool test/test.dot
```

## Grammar

```ebnf
d = 'digraph' ID g                  (* digraph *)
g = '{' S '}'                       (* subgraph *)
S = s ... s                         (* statement list *)
s = ','
  | ';'
  | (ID | g) '->' ... '->' (ID | g) (* edge statement *)
```
where
* `ID` = [`a`-`zA`-`Z`][`a`-`zA`-`Z0`-`9`]*

In addition, GraphTool supports
* `/* C-style */` and
* `// C++-style` comments.

## Entry \& Exit

The first node mentioned is considered the *entry*, the last one the *exit*.

## Caveats

Right now, GraphTool can't handle graphs with unreachable nodes.
