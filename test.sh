#!/bin/bash

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
