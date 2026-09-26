#!/usr/bin/env bash
#
# GraphTool's test suite: a golden-file driver.
#
# Every input in this directory yields two cases -- `<name>` and `<name>-c` (the same input with
# `-c`, which eliminates critical edges first). A case copies its input into a directory of its own
# below the build tree -- `build/test/<case>/` unless `--out` says otherwise -- runs GraphTool
# there, and diffs the six results against the blessed copies `golden/<case>.<suffix>.dot`. Running
# in the build tree keeps the results out of the source tree and leaves them around for inspection.
# `--bless` refreshes the blessed copies -- inspect the resulting diff before committing it.
#
# CI and `ctest`/`make test` call this script, so it is also the thing to run by hand:
#     test/run.sh                  # all cases
#     test/run.sh cytron cytron-c  # just these two
#     test/run.sh --valgrind       # additionally demand a clean Valgrind report
#
# Deliberately POSIX-ish bash: the macOS runner's /bin/bash is still 3.2.

set -u

self=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
root=$(dirname -- "$self")

# What `main.cpp` writes next to the input, and hence what `golden/<case>.<suffix>.dot` holds.
suffixes="forward backward dom_tree postdom_tree dom_frontiers postdom_frontiers"

exe=${GRAPHTOOL:-$root/build/bin/graphtool}
out=
bless=0
list=0
use_valgrind=0

usage() {
    cat <<EOF
Usage: test/run.sh [options] [<case>...]

Runs GraphTool over the inputs in test/ and diffs the results against test/golden/.
Without <case>s, every case runs; a <case> is an input's name, optionally suffixed
with \`-c\` for the run with critical-edge elimination, e.g. \`cytron\` or \`cytron-c\`.

Options:
  -e, --exe <path>  GraphTool binary to test [default: \`\$GRAPHTOOL\`, else
                    \`build/bin/graphtool\`]
  -o, --out <dir>   Where to run the cases; each one gets a \`<dir>/<case>/\`
                    [default: \`test\` next to the binary's build directory, i.e.
                    \`build/test\`]
  -b, --bless       Overwrite the golden files with what GraphTool produces now.
  -g, --valgrind    Run each case under Valgrind and fail on any report.
  -l, --list        List all case names and exit.
  -h, --help        Display this help and exit.
EOF
}

die() {
    echo "error: $*" >&2
    exit 2
}

while [ $# -gt 0 ]; do
    case $1 in
        -e|--exe)      [ $# -ge 2 ] || die "\`$1\` needs an argument"; exe=$2; shift 2;;
        -o|--out)      [ $# -ge 2 ] || die "\`$1\` needs an argument"; out=$2; shift 2;;
        -b|--bless)    bless=1;       shift;;
        -g|--valgrind) use_valgrind=1; shift;;
        -l|--list)     list=1;        shift;;
        -h|--help)     usage; exit 0;;
        --)            shift; break;;
        -*)            die "unknown option \`$1\`";;
        *)             break;;
    esac
done

# `test/unreachable.dot` crashes GraphTool -- see the caveat in README.md. Enumerating the skips
# here rather than in the CI workflows is the point of this script: every runner skips the same set.
skip() {
    case ${1%-c} in
        unreachable) return 0;;
    esac
    return 1
}

# An input is `<name>.dot`; `<name>.dot.<suffix>.dot` is a result of an earlier run by hand.
all_cases() {
    for input in "$self"/*.dot; do
        case $input in *.dot.*.dot) continue;; esac
        name=$(basename "$input" .dot)
        skip "$name" && continue
        echo "$name"
        echo "$name-c"
    done
}

if [ $list -eq 1 ]; then
    all_cases
    exit 0
fi

if [ $# -eq 0 ]; then
    cases=$(all_cases)
else
    cases=$(printf '%s\n' "$@")
    for name in $cases; do
        [ -e "$self/${name%-c}.dot" ] || die "no input \`test/${name%-c}.dot\` for case \`$name\`"
        skip "$name" && die "case \`$name\` is on the skip list"
    done
fi

[ -x "$exe" ] || die "no GraphTool binary at \`$exe\` -- build it or pass \`--exe\`"
if [ $use_valgrind -eq 1 ]; then
    command -v valgrind > /dev/null || die "\`--valgrind\` given, but Valgrind is not installed"
fi

# The binary lives in `<build>/bin/`, so its build directory is two levels up; that is where the
# results go. `ctest -j` is safe because each case owns its directory.
[ -n "$out" ] || out=$(dirname -- "$(dirname -- "$exe")")/test
mkdir -p "$out" || die "cannot create the output directory \`$out\`"
out=$(cd -- "$out" && pwd)

if [ -t 1 ]; then
    red=$(printf '\033[31m'); green=$(printf '\033[32m'); bold=$(printf '\033[1m'); off=$(printf '\033[m')
else
    red=; green=; bold=; off=
fi

reason=  # what made the case at hand fail
note() {
    echo "    $*" >&2
    reason=${reason:-$1}
}

# Diffs `$1` against `$2` ignoring CRLF, which is what MSVC's `ofstream` writes in text mode.
diff_lf() {
    tr -d '\r' < "$1" > "$work/lhs"
    tr -d '\r' < "$2" > "$work/rhs"
    diff -u -L "$3" -L "$4" "$work/lhs" "$work/rhs"
}

run_case() {
    name=$1
    input=${name%-c}.dot
    work=$out/$name
    golden=$self/golden/$name
    reason=

    # Start from an empty directory: a result left over from an earlier run would otherwise pass
    # for one GraphTool has just written.
    rm -rf "$work" && mkdir -p "$work" || return 1
    cp "$self/$input" "$work/$input" || return 1

    set -- "$work/$input"
    [ "$name" = "${name%-c}" ] || set -- "$@" -c

    if [ $use_valgrind -eq 1 ]; then
        # No `--error-exitcode`, so that the exit status remains GraphTool's own; the log is the
        # verdict on the memory. Demanding a "0 errors" summary rather than grepping for errors
        # also fails closed if Valgrind dies before it gets to report anything.
        valgrind --leak-check=full --log-file="$work/valgrind.log" "$exe" "$@" > "$work/out" 2>&1
        status=$?
    else
        "$exe" "$@" > "$work/out" 2>&1
        status=$?
    fi

    if [ $status -ne 0 ]; then
        note "exited with $status, expected 0"
        [ -s "$work/out" ] && sed 's/^/    | /' "$work/out" >&2
    fi

    if [ $use_valgrind -eq 1 ] && ! grep -q "ERROR SUMMARY: 0 errors" "$work/valgrind.log"; then
        note "Valgrind reported errors"
        sed 's/^/    | /' "$work/valgrind.log" >&2
    fi

    if [ $bless -eq 1 ]; then
        [ -n "$reason" ] && return 1
        for suffix in $suffixes; do
            cp "$work/$input.$suffix.dot" "$golden.$suffix.dot" || note "GraphTool wrote no \`$suffix\`"
        done
        [ -n "$reason" ] && return 1
        return 0
    fi

    for suffix in $suffixes; do
        if [ ! -e "$golden.$suffix.dot" ]; then
            note "no golden file \`test/golden/$name.$suffix.dot\` -- run \`test/run.sh --bless\`"
        elif [ ! -e "$work/$input.$suffix.dot" ]; then
            note "GraphTool wrote no \`$suffix\`"
        elif ! diff_lf "$golden.$suffix.dot" "$work/$input.$suffix.dot" \
                       "golden/$name.$suffix.dot" "$input.$suffix.dot" >&2; then
            note "\`$suffix\` differs from its golden file"
        fi
    done

    [ -z "$reason" ]
}

passed=0
failed=
for name in $cases; do
    if run_case "$name"; then
        [ $bless -eq 1 ] && echo "${green}blessed${off} $name" || echo "${green}ok${off}      $name"
        passed=$((passed + 1))
    else
        echo "${red}${bold}FAILED${off}  $name: $reason"
        failed="$failed $name"
    fi
done

if [ -n "$failed" ]; then
    echo "${red}${bold}$passed passed, $(printf '%s\n' $failed | wc -l | tr -d ' ') failed:${off}$failed"
    echo "results in: $out"
    exit 1
fi

[ $bless -eq 1 ] && echo "${green}${bold}$passed blessed${off}" || echo "${green}${bold}$passed passed${off}"
