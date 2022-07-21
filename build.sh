#!/bin/bash
# Compiles the wtk library and the examples.

cmd() {
    [ $VERBOSE -eq 1 ] && echo "$@"
    [ $TOUCH -eq 0 ] && $@
}

usage() {
    echo "$0 [--option[=<value>]]"
    echo ""
    echo "Possible options are:"
    echo "  --backend=<value>   Compile wtk with the specified backend."
    echo "                          <value> can be: cocoa"
    echo "  --no-examples       Only compile the library"
    echo "  --verbose           Show the commands as they are run"
    echo "  --touch             Show the commands that will be run, but don't actually execute anything"
    echo "  --release           Compile with release flags"
    echo "  --help              Show this message and exit"
}

BACKEND=""
EXAMPLES=1
VERBOSE=0
TOUCH=0
RELEASE=0

for arg in "$@"; do
    case "$arg" in
        --backend=*)        BACKEND="${arg#*=}";;
        --no-examples)      EXAMPLES=0;;
        --verbose|--touch)  VERBOSE=1;;
        --touch)            TOUCH=1;;
        --release)          RELEASE=1;;
        --help|*)           usage && [ "$arg" == "--help" ] && exit 0 || exit 1;;
    esac
done

# Try to select a backend if none was specified

UNAME=$(uname -s)
if [ -z $BACKEND ]; then
    if [ "$UNAME" == "Darwin" ]; then
        BACKEND="cocoa"
    else
        echo "ERROR: Unknown platform." && exit 1
    fi
fi

CFLAGS="-std=c99 -Wall -Iinclude"
if [ ! $RELEASE ]; then
    CFLAGS="$CFLAGS -Werror -g3"
else
    CFLAGS="$CFLAGS -O2"
fi

echo -n "INFO: Compiling bin/libwtk.a with the $BACKEND backend... "

cmd mkdir -p bin
cmd gcc $CFLAGS -c source/$BACKEND.?
cmd ar crs bin/libwtk.a $BACKEND.o
cmd rm $BACKEND.o

echo "Done."
[ $EXAMPLES -eq 0 ] && exit 0

LFLAGS="-Lbin -lwtk"
if [ $BACKEND == "cocoa" ]; then
    LFLAGS="$LFLAGS -framework Cocoa -framework OpenGL"
fi

for f in examples/*.c; do
    ff=$(basename $f .c)
    echo -n "INFO: Compiling bin/$ff... "
    cmd gcc $CFLAGS $f -o bin/$ff $LFLAGS
    echo "Done."
done
