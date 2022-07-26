#!/bin/bash

##############################################################################

function error() {
    echo -e "\033[0;31mError: $1\033[0m\n";
}

function usage() {
    echo "Usage: $0 [--option[=<value>]]"
    echo "    --backend=<value>     Compile with the specified backend. One of: [cocoa]"
    echo "    --examples            Compile examples"
    echo "    --release             Compile with release flags"
}

# Parse args
for i in "$@"; do case $i in
    --backend=*)                    eval "${i##*--}";;
    --examples|--release|--help)    eval "${i##*--}=1";;
    *)                              error "Unknown option: $i" && usage && exit 1;;
esac; done

[[ -n "$help" ]] && usage && exit 0

##############################################################################

CFLAGS="-std=c99 -Wall -Iinclude"
LFLAGS="-Lbin -lwtk"

if [[ -z "$backend" ]]; then
    UNAME="$(uname -s)"
    if [[ $UNAME == "Darwin" ]]; then
        backend="cocoa"
    else
        error "Unsupported platform." && exit 1
    fi
fi

if [[ "$backend" == "cocoa" ]]; then
    LFLAGS="$LFLAGS -framework Cocoa -framework OpenGL"
fi

if [[ -z "$release" ]]; then
    CFLAGS="$CFLAGS -Wextra -Werror -g3 -DDEBUG"
else
    CFLAGS="$CFLAGS -O3 -DNDEBUG"
fi

##############################################################################

echo -n "Compiling bin/libwtk.a with the $backend backend... "

mkdir -p bin
gcc $CFLAGS -c source/$backend.?
ar crs bin/libwtk.a $backend.o
rm $backend.o

echo "Done."
[[ -z "${examples}" ]] && exit 0

for f in examples/*.c; do
    ff=$(basename $f .c)
    echo -n "Compiling bin/$ff... "
    cc $CFLAGS $f -o bin/$ff $LFLAGS
    echo "Done."
done

##############################################################################