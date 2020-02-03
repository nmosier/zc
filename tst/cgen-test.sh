#!/bin/sh

USAGE="usage: $0 test_dir command [arguments...]"

if [[ $# -lt 2 ]]; then
    echo "$USAGE"
    exit 1
fi

TEST_DIR="$1"
shift 1

OUT_DIR=$(mktemp -d 2> /dev/null || mktemp -d -t 'tmp') || exit 1
trap "rm -rf $OUT_DIR" 0

cd "$TEST_DIR"

tests=( $(find . -name "*.c") )

# Track number of passing tests
total=0
passed=0

# $1: test name
# $2: error string
fail()
{
    echo "failed: '$1': $2"
}

for test in "${tests[@]}"; do
    (( ++total ))
    name=$(basename "$test")
    OUT_ASM="$(basename "$test" .c)".z80
    OUT_8XP="$(basename "$test" .c)".8xp

    # code generation
    if ! "$@" -o "$OUT_ASM" "$test" >/dev/null; then
        fail "$test" "code generation failed"
        continue
    fi

    # assemble
    if ! [ -f "$test".z80 ]; then
        fail "$test" "missing z80 wrapper file '$test.z80'"
        continue
    fi


    SPASM_ERR="$OUT_DIR/${test}.spasm" 
    if ! spasm -E "$test".z80 "$OUT_8XP" > "$SPASM_ERR"; then
        fail "$test" "assembler failed"
        cat "$SPASM_ERR"
        continue
    fi

    # run autotester
    if ! cemu-autotester "$test".json >/dev/null; then
        fail "$test" "autotester failed"
        continue
    fi

    (( ++passed ))
done

echo "Total tests: $total"
echo "Passing tests: $passed"
echo "Failing tests: $((total - passed))"

exit $( [[ $passed -eq $total ]] )
