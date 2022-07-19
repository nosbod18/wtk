#!/bin/bash

CFLAGS="-std=c99 -Wall -Wextra -Wpedantic -Werror"
BACKEND="null"
UNAME=$(uname -s)

for arg in "$@"; do
    case "$arg" in
        --backend=*) BACKEND="$(echo $arg | sed 's/--backend=//')";;
    esac
done

if [ $BACKEND == "null" ]; then
    if [ "$UNAME" == "Darwin" ]; then
        BACKEND="cocoa"
    elif [ "$UNAME" == "Linux" ]; then
        BACKEND="x11"
    else
        echo "WARNING: Compiling with a null backend"
    fi
fi

echo "INFO: Compiling wtk using the $BACKEND backend..."

gcc $CFLAGS -c source/$BACKEND.? -o source/$BACKEND.o\
    && ar crs libwtk.a source/$BACKEND.o\
    && rm source/$BACKEND.o

echo "INFO: Done"

