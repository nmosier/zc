#!/bin/bash

# BENCHMARK FOR ZC

USAGE="$0 BENCHMARK_DIR PARAMS_FILE OUTFILE"

if [[ $# -lt 3 ]]; then
    echo "$USAGE"
    exit 1
fi

DIR="$1"
PARAMSF="$2"
OUTF="$3"

if ! [ -d "$DIR" ]; then
    echo "$0: $DIR: directory does not exist"
    exit 2
fi
SRCS="$DIR"/*.c

if ! [ -f "$PARAMSF" ]; then
    echo "$0: $PARAMSF: file does not exist"
    exit 2
fi
# PARAMS=$(< "$PARAMSF")

if ! [ -d "$(dirname "$OUTF")" ]; then
    echo "$0: " $(dirname "$OUTF"): directory does not exist
    exit 2
fi
if [ -f "$OUTF" ]; then
    rm "$OUTF"
fi

# $1: SRC
# $2: PARAM
getsize() {
    BIN="$(dirname $1)"/bare-"$(basename $1 .c)".bin
    rm -f "$BIN"
    if ! make -B "$BIN" $2 > /dev/null 2> /dev/null; then
        echo "encountered error testing \"$1\" on \"$2\"" >&2
        return 1
    fi
    wc -c < "$BIN"
}

PARAM1=$(head -n1 "$PARAMSF")
# PARAM_ARR=($PARAMS)
# PARAM1=${PARAM_ARR[0]}


for SRC in $SRCS; do
    echo "$SRC" >> "$OUTF"
    # LEGEND
    printf "NAME\tTOTAL\tABS-DIFF\tPCT-DIFF\n" >> "$OUTF"
    REFSIZE=$(getsize "$SRC" "$PARAM1") || exit 1
    while read PARAM; do
    # for PARAM in $PARAMS; do
        SIZE=$(getsize "$SRC" "$PARAM") || continue
        DIFF=$(($SIZE - $REFSIZE))
        PCTDIFF=$(awk "BEGIN {print ${DIFF}*100/${REFSIZE}}")
        printf "\t%s\t%d\t%d\t%.1f\n" $PARAM $SIZE $DIFF $PCTDIFF  >> "$OUTF"
    done < "$PARAMSF"
    printf "\n" >> "$OUTF"
done
