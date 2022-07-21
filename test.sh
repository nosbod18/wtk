#!/bin/bash
# Compiles the files in the example directory.
# You can specify a filename as a parameter to only compile and run that example.
# E.g. `./test.sh 01-simple.c` will only compile examples/01-simple.c and it will
# run it when it's finished compiling

FILES="examples/${1:-*.c}"
CFLAGS="-std=c99 -Wall -Wextra -Wpedantic"
LFLAGS="-Lbin -lwtk"
UNAME=$(uname -s)

if [ $UNAME == "Darwin" ]; then
    LFLAGS="$LFLAGS -framework Cocoa -framework OpenGL"
elif [ $UNAME == "Linux" ]; then
    LFLAGS="$LFLAGS -lX11 -lGL"
fi

mkdir -p bin
for f in $FILES; do
    gcc $CFLAGS $f -o bin/$(basename $f .c) $LFLAGS
done

[ "$1" ] && ./bin/$(basename $1 .c)
