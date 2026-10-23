#!/usr/bin/env bash
# Conch test suite: pipes, redirects and builtins.
#
# Usage:
#   tests/run_tests.sh [path/to/conch]             # functional tests
#   tests/run_tests.sh --valgrind [path/to/conch]  # same tests under valgrind
#
# Each test feeds a script to conch on stdin and checks the files it writes
# or distinctive tokens in its output. Tests marked xfail document known
# limitations: they are expected to fail and do not affect the exit status,
# but are reported as XPASS if they start passing.

set -u

VALGRIND=0
if [ "${1:-}" = "--valgrind" ]; then
    VALGRIND=1
    shift
fi

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CONCH="${1:-$ROOT/bin/conch}"
case "$CONCH" in /*) ;; *) CONCH="$PWD/$CONCH" ;; esac

if [ ! -x "$CONCH" ]; then
    echo "conch binary not found at $CONCH (run 'make' first)" >&2
    exit 2
fi

VALGRIND_CMD=(valgrind --leak-check=full --show-leak-kinds=all
              --errors-for-leak-kinds=definite,indirect,possible
              --child-silent-after-fork=yes --error-exitcode=99)
TIMEOUT=10
if [ "$VALGRIND" -eq 1 ]; then
    command -v valgrind >/dev/null || { echo "valgrind not installed" >&2; exit 2; }
    TIMEOUT=60
fi

TMP="$(mktemp -d "${TMPDIR:-/tmp}/conch-tests.XXXXXX")"
trap 'rm -rf "$TMP"' EXIT

PASS=0 FAIL=0 XFAIL=0 XPASS=0
FAILED_NAMES=()
CUR_OK=1
VG_BAD=0
VG_SUMMARY=""

# Run conch with the given script on stdin, inside a fresh work dir.
# Output (stdout+stderr, ANSI codes stripped) lands in $OUT.
run_conch() {
    local script="$1"
    WORK="$TMP/$TEST_NAME"
    mkdir -p "$WORK/home"
    OUT="$WORK/.out"
    local vg_log="$WORK/.valgrind.log"
    local runner=()
    [ "$VALGRIND" -eq 1 ] && runner=("${VALGRIND_CMD[@]}" "--log-file=$vg_log")

    printf '%s\n' "$script" |
        (cd "$WORK" && HOME="$WORK/home" perl -e 'alarm shift; exec @ARGV' \
            "$TIMEOUT" ${runner[@]+"${runner[@]}"} "$CONCH") > "$OUT.raw" 2>&1
    RC=$?
    perl -pe 's/\e\[[0-9;]*[A-Za-z]//g' "$OUT.raw" > "$OUT"

    if [ "$RC" -eq 142 ]; then
        note "timed out after ${TIMEOUT}s"
    fi
    if [ "$VALGRIND" -eq 1 ]; then
        if [ "$RC" -eq 99 ] || ! grep -q "ERROR SUMMARY: 0 errors" "$vg_log"; then
            note "valgrind reported errors (log below)"
            VG_BAD=1
            sed 's/^/        | /' "$vg_log"
        fi
        VG_SUMMARY+="$TEST_NAME: $(grep -E 'in use at exit|ERROR SUMMARY' "$vg_log" | sed 's/^==[0-9]*== //' | tr '\n' ' ')"$'\n'
    fi
}

note() { echo "      - $*"; CUR_OK=0; }

# File $1 must exist with exactly the content $2 (trailing newline implied).
expect_file() {
    local path="$WORK/$1" want
    want="$(printf '%s\n' "$2")"
    if [ ! -f "$path" ]; then
        note "expected file $1 to exist"
    elif [ "$(cat "$path")" != "$want" ]; then
        note "file $1: expected [$(echo "$want" | tr '\n' '|')], got [$(tr '\n' '|' < "$path")]"
    fi
}

expect_no_file() {
    if [ -e "$WORK/$1" ]; then
        note "expected file $1 not to exist"
    fi
}

expect_out() {
    if ! grep -qF -- "$1" "$OUT"; then
        note "expected output to contain [$1]"
    fi
}

# Output must contain the multi-line sequence $1 (a perl regex). Not anchored
# to line starts: with stdout not a tty, conch's echoed input line is still
# buffered when the child writes, so the first output line can be glued to it.
expect_out_seq() {
    if ! perl -0ne "exit !/$1/" "$OUT"; then
        note "expected output to contain $2"
    fi
}

expect_no_out() {
    if grep -qF -- "$1" "$OUT"; then
        note "expected output not to contain [$1]"
    fi
}

# run_test NAME [xfail] -- runs function t_NAME and records the result.
run_test() {
    local name="$1" mode="${2:-}" log="$TMP/$1.log"
    TEST_NAME="$name"
    CUR_OK=1
    VG_BAD=0
    "t_$name" > "$log"

    if [ "$mode" = "xfail" ]; then
        if [ "$CUR_OK" -eq 1 ]; then
            echo "XPASS $name (known limitation now works; update README and drop xfail)"
            XPASS=$((XPASS + 1))
        else
            echo "XFAIL $name"
            XFAIL=$((XFAIL + 1))
        fi
        # Memory errors count even for xfail tests.
        if [ "$VG_BAD" -eq 1 ]; then
            cat "$log"
            FAIL=$((FAIL + 1))
            FAILED_NAMES+=("$name(valgrind)")
        fi
    elif [ "$CUR_OK" -eq 1 ]; then
        echo "PASS  $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL  $name"
        cat "$log"
        FAIL=$((FAIL + 1))
        FAILED_NAMES+=("$name")
    fi
}

# ---------------------------------------------------------------- startup/exit

t_eof_exits() {
    run_conch 'echo started > s.txt'
    expect_file s.txt "started"
    [ "$RC" -eq 0 ] || note "expected exit status 0 on EOF, got $RC"
}

t_exit_stops_processing() {
    run_conch 'echo before > a.txt
exit
echo after > b.txt'
    expect_file a.txt "before"
    expect_no_file b.txt
    expect_out "CONCH SESSION TERMINATED"
}

# ---------------------------------------------------------------- external commands

t_external_command() {
    run_conch 'printf "%s-%s\n" alpha beta > out.txt'
    expect_file out.txt "alpha-beta"
}

t_command_not_found() {
    run_conch 'definitely_not_a_command_xyz'
    expect_out "conch: COMMAND NOT FOUND"
}

t_double_quotes() {
    run_conch 'printf "[%s]\n" "two words" > out.txt'
    expect_file out.txt "[two words]"
}

t_single_quotes() {
    run_conch "printf '[%s]\n' 'two words' > out.txt"
    expect_file out.txt "[two words]"
}

t_variable_expansion() {
    run_conch 'export GREETING=hello
echo $GREETING ${GREETING}world > out.txt'
    expect_file out.txt "hello helloworld"
}

# ---------------------------------------------------------------- redirects

t_redirect_out() {
    run_conch 'echo first > out.txt
echo second > out.txt'
    expect_file out.txt "second"
}

t_redirect_append() {
    run_conch 'echo one > out.txt
echo two >> out.txt'
    expect_file out.txt "one
two"
}

t_redirect_in() {
    run_conch 'printf "c\na\nb\n" > in.txt
sort < in.txt'
    expect_file in.txt "c
a
b"
    expect_out_seq 'a\nb\nc\n' "sorted a, b, c"
}

t_redirect_stderr() {
    run_conch 'ls /no/such/path/xyz 2> err.txt'
    if ! grep -q "xyz" "$WORK/err.txt" 2>/dev/null; then
        note "expected ls error message in err.txt"
    fi
}

t_redirect_bad_input_file() {
    run_conch 'cat < missing.txt
echo still-alive > alive.txt'
    expect_out "conch: OPEN"
    expect_file alive.txt "still-alive"
}

t_redirect_missing_filename() {
    run_conch 'echo oops >
echo still-alive > alive.txt'
    expect_out "conch: syntax error: missing filename after '>'"
    expect_file alive.txt "still-alive"
}

# ---------------------------------------------------------------- pipes

t_pipe_simple() {
    run_conch 'echo hello | tr a-z A-Z'
    expect_out "HELLO"
}

t_pipe_sort() {
    run_conch 'printf "zeta\nalpha\n" | sort'
    expect_out_seq 'alpha\nzeta\n' "alpha before zeta"
}

t_pipe_large_output() {
    run_conch 'seq 1 20000 | wc -l'
    expect_out "20000"
}

t_pipe_missing_command() {
    run_conch 'echo hi | not_a_real_cmd_xyz
echo still-alive > alive.txt'
    expect_out "conch: COMMAND 2 NOT FOUND"
    expect_file alive.txt "still-alive"
}

# ---------------------------------------------------------------- builtins

t_builtin_cd() {
    run_conch 'mkdir sub
cd sub
pwd > here.txt'
    expect_file sub/here.txt "$(cd "$WORK/sub" && pwd -P)"
}

t_builtin_cd_home() {
    run_conch 'cd
pwd > here.txt'
    if [ ! -f "$WORK/home/here.txt" ]; then
        note "expected bare cd to go to \$HOME"
    fi
}

t_builtin_cd_missing() {
    run_conch 'cd /no/such/dir/xyz'
    expect_out "conch: CD"
}

t_builtin_export() {
    run_conch 'export CONCH_TEST_VAR=exported
printenv CONCH_TEST_VAR > out.txt'
    expect_file out.txt "exported"
}

t_builtin_export_list() {
    run_conch 'export CONCH_LIST_VAR=listed
export'
    expect_out "export CONCH_LIST_VAR=listed"
}

t_builtin_help() {
    run_conch 'help'
    expect_out "CONCH HELP"
    expect_out "Built-in commands:"
    for b in cd help exit clear status jobs fg bg export; do
        grep -qx "  $b" "$OUT" || note "help does not list builtin $b"
    done
}

t_builtin_status() {
    run_conch 'status'
    expect_out "SYSTEM STATUS: OPERATIONAL"
    expect_out "CONCH VERSION:"
    expect_out "CURRENT DIR: "
}

t_builtin_clear() {
    run_conch 'clear'
    # boot banner appears twice: once at startup, once from clear
    local n
    n="$(grep -c "CONCH SHELL" "$OUT")"
    [ "$n" -ge 2 ] || note "expected clear to reprint the boot banner"
}

t_builtin_jobs_fg() {
    run_conch 'sleep 1 &
jobs
fg %1
echo done > after.txt
jobs'
    expect_out "[1] "
    expect_out "Running"
    expect_out "sleep 1"
    expect_file after.txt "done"
}

t_builtin_bg_running() {
    run_conch 'sleep 1 &
bg %1
fg %1'
    expect_out "conch: bg: job already running"
}

t_builtin_fg_bg_no_job() {
    run_conch 'fg %9
bg %9'
    expect_out "conch: fg: no such job"
    expect_out "conch: bg: no such job"
}

t_background_done() {
    run_conch 'sleep 0 &
sleep 1
echo tick > tick.txt'
    expect_out "Done"
    expect_file tick.txt "tick"
}

# ---------------------------------------------------------------- known limitations (xfail)

t_xfail_multi_pipe() {
    run_conch 'echo abc | tr a x | tr b y > out.txt'
    expect_out "xyc"
}

t_xfail_pipe_with_redirect() {
    run_conch 'echo hi | cat > out.txt'
    expect_file out.txt "hi"
}

t_xfail_in_and_out_redirect() {
    run_conch 'echo data > in.txt
cat < in.txt > out.txt'
    expect_file out.txt "data"
}

t_xfail_args_after_redirect() {
    run_conch 'printf "a\nb\n" > in.txt
sort < in.txt -r > out.txt'
    expect_file out.txt "b
a"
}

t_xfail_redirect_without_space() {
    run_conch 'echo hi >out.txt'
    expect_file out.txt "hi"
}

t_xfail_semicolon() {
    run_conch 'echo one > a.txt ; echo two > b.txt'
    expect_file a.txt "one"
    expect_file b.txt "two"
}

t_xfail_and_list() {
    run_conch 'true && echo yes > out.txt'
    expect_file out.txt "yes"
}

t_xfail_single_quotes_literal() {
    run_conch "export Q=expanded
printf '%s\n' '\$Q' > out.txt"
    expect_file out.txt '$Q'
}

t_xfail_adjacent_quotes() {
    run_conch 'printf "%s\n" foo"bar baz" > out.txt'
    expect_file out.txt "foobar baz"
}

# ---------------------------------------------------------------- main

echo "conch: $CONCH"
[ "$VALGRIND" -eq 1 ] && echo "valgrind: ${VALGRIND_CMD[*]}"
echo

for t in eof_exits exit_stops_processing \
         external_command command_not_found double_quotes single_quotes variable_expansion \
         redirect_out redirect_append redirect_in redirect_stderr redirect_bad_input_file redirect_missing_filename \
         pipe_simple pipe_sort pipe_large_output pipe_missing_command \
         builtin_cd builtin_cd_home builtin_cd_missing builtin_export builtin_export_list \
         builtin_help builtin_status builtin_clear builtin_jobs_fg builtin_bg_running \
         builtin_fg_bg_no_job background_done; do
    run_test "$t"
done

for t in xfail_multi_pipe xfail_pipe_with_redirect xfail_in_and_out_redirect \
         xfail_args_after_redirect xfail_redirect_without_space xfail_semicolon \
         xfail_and_list xfail_single_quotes_literal xfail_adjacent_quotes; do
    run_test "$t" xfail
done

echo
echo "passed: $PASS  failed: $FAIL  xfail: $XFAIL  xpass: $XPASS"
if [ "$VALGRIND" -eq 1 ]; then
    echo
    echo "valgrind summary per test:"
    printf '%s' "$VG_SUMMARY"
fi
if [ "$FAIL" -gt 0 ]; then
    echo "failed tests: ${FAILED_NAMES[*]}"
    exit 1
fi
exit 0
