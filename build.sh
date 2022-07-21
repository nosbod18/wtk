#!/bin/bash
# Compiles the wtk library into an archive.

cmd() {
    [ $VERBOSE -eq 1 ] && echo "$@"
    [ $TOUCH -eq 0 ] && $@
}

usage() {
    echo "$0 [--option[=<value>]]"
    echo ""
    echo "Possible options are:"
    echo "  --help              Show this message and exit"
    echo "  --verbose           Show the commands as they are run"
    echo "  --touch             Show the commands that will be run,"
    echo "                          but don't actually execute anything"
    echo "  --backend=<value>   Compile wtk with the specified backend."
    echo "                          <value> can be: cocoa, xlib, xcb, wayland, or win32"
}

BACKEND="null"
VERBOSE=0
TOUCH=0

for arg in "$@"; do
    case "$arg" in
        --verbose)
            VERBOSE=1;;

        --touch)
            TOUCH=1
            VERBOSE=1;;

        --backend=*)
            BACKEND="${arg#*=}";;

        --help|*)
            usage
            [ "$arg" == "--help" ] && exit 0 || exit 1;;
    esac
done

UNAME=$(uname -s)
if [ $BACKEND == "null" ]; then
    if [ "$UNAME" == "Darwin" ]; then
        BACKEND="cocoa"
    elif [ "$UNAME" == "Linux" ]; then
        BACKEND="xlib"
    else
        echo "WARNING: Unknown platform."
    fi
fi

CFLAGS="-std=c99 -Wall -Wextra -Wpedantic"

echo "INFO: Compiling bin/libwtk.a using the $BACKEND backend..."

cmd mkdir -p bin
cmd gcc $CFLAGS -c source/$BACKEND.? -o source/$BACKEND.o
cmd ar crs bin/libwtk.a source/$BACKEND.o
cmd rm source/$BACKEND.o

echo "INFO: Done."
