# GraphTool

![linux](https://img.shields.io/github/actions/workflow/status/leissa/graphtool/linux.yml?logo=linux&logoColor=white&label=linux&link=https%3A%2F%2Fgithub.com%2Fleissa%2Fgraphtool%2Factions%2Fworkflows%2Flinux.yml)
![macos](https://img.shields.io/github/actions/workflow/status/leissa/graphtool/macos.yml?logo=apple&logoColor=white&label=macos&link=https%3A%2F%2Fgithub.com%2Fleissa%2Fgraphtool%2Factions%2Fworkflows%2Fmacos.yml)
![windows](https://img.shields.io/github/actions/workflow/status/leissa/graphtool/windows.yml?logo=windows&logoColor=white&label=windows&link=https%3A%2F%2Fgithub.com%2Fgraphtool%2Fleissa%2Factions%2Fworkflows%2Fwindows.yml)

A small tool that reads a subset from [Graphviz'](https://graphviz.org) [DOT language](https://graphviz.org/doc/info/lang.html) and calculates several [dominance-related](https://en.wikipedia.org/wiki/Dominator_(graph_theory)) properties:
* dominance tree
* postdominance tree
* dominance frontiers
* postdominance tree frontieres (aka control dependence)

## Usage

```
USAGE:
  graphtool [-?|-h|--help] [-v|--version] [<file>]

Display usage information.

OPTIONS, ARGUMENTS:
  -?, -h, --help
  -v, --version           Display version info and exit.
  <file>                  Input file.
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
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```
For a `Release` build simply use `-DCMAKE_BUILD_TYPE=Release`.

Invoke the interpreter like so:
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

## Entry \& Exit

The first node mentioned is considered the *entry*, the last one the *exit.
