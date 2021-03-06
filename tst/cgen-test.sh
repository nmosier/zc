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

COMMAND=$@

# $1 -- test
run_test() {
    test="$1"
    name="$(basename $test)"
    OUT_ASM="${test/%.c/.z80}"
    OUT_8XP="${test/%.c/.8xp}"

    # code generation
    if ! $COMMAND -o "$OUT_ASM" "$test" >/dev/null 2>&1; then
        fail "$test" "code generation failed"
        return 1
    fi

    # assemble
    if ! [ -f "$test".z80 ]; then
        fail "$test" "missing z80 wrapper file '$test.z80'"
        return 1
    fi


    SPASM_ERR="$OUT_DIR/${name}.spasm"
    if ! spasm -E -L -I. -I"$(dirname $test)" "$test".z80 "$OUT_8XP" > "$SPASM_ERR"; then
        fail "$test" "assembler failed"
        cat "$SPASM_ERR"
        return 1
    fi

    # run autotester
    AUTO_ERR="$OUT_DIR/${name}.auto"
    if ! cemu-autotester "$test".json >"$AUTO_ERR"; then
        fail "$test" "autotester failed"
        cat "$AUTO_ERR"
        return 1
    fi

    (( ++passed ))
    
    return 0
}

for test in "${tests[@]}"; do
    (( ++total ))
    run_test "$test" > "$OUT_DIR/$total.out" &
done

wait

total=0
for test in "${tests[@]}"; do
    (( ++total ))
    if [ -s "$OUT_DIR/$total.out" ]; then
        cat "$OUT_DIR/$total.out"
    else
        (( ++passed ))
    fi
done

echo "Total tests: $total"
echo "Passing tests: $passed"
echo "Failing tests: $((total - passed))"

exit $( [[ $passed -eq $total ]] )
