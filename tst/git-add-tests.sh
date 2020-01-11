#!/bin/sh

DIRS="semant cgen"
for DIR in $DIRS; do
    git add $DIR/*.test{,.stderr}
done
