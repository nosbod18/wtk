#!/bin/bash
cd "${0%/*}"
mkdir -p bin

# Build library
gcc -c src/wtk.c -o bin/wtk.o
ar rcs bin/libwtk.a bin/wtk.o
rm bin/wtk.o

# Build demo
if [[ "$(uname -s)" == "Darwin" ]]; then
    gcc demo/main.c -o bin/demo -Lbin -lwtk -framework Cocoa -framework OpenGL
else
    gcc demo/main.c -o bin/demo -Lbin -lwtk -lX11 -lGL
fi

