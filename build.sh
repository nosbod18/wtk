#!/bin/bash

CFLAGS="-std=c99 -Wall -Wextra -Werror -Iinclude -Isource -g3"
LFLAGS="-Lbin -lwtk -lX11 -lGL"

mkdir -p bin
gcc $CFLAGS -c source/wtk.c -o bin/wtk.o
ar crs bin/libwtk.a bin/wtk.o
rm bin/wtk.o

gcc $CFLAGS examples/01-simple.c -o bin/01-simple $LFLAGS

# ##############################################################################

# function error() {
#     echo -e "\033[0;31mError: $1\033[0m\n";
# }

# function usage() {
#     echo "Usage: $0 [--option[=<value>]]"
#     echo "    --backend=<value>     Compile with the specified backend. One of: [cocoa, x11]"
#     echo "    --examples            Compile examples"
#     echo "    --release             Compile with release flags"
# }

# # Parse arguments
# for i in "$@"; do case $i in
#     --backend=*)                    eval "${i##*--}";;
#     --examples|--release|--help)    eval "${i##*--}=1";;
#     *)                              error "Unknown option: $i" && usage && exit 1;;
# esac; done

# [[ -n "$help" ]] && usage && exit 0

# ##############################################################################

# CFLAGS="-std=c99 -Wall -Iinclude"
# LFLAGS="-Lbin -lwtk"

# if [[ -z "$backend" ]]; then
#     UNAME="$(uname -s)"
#     if [[ $UNAME == "Darwin" ]]; then
#         backend="cocoa"
#     elif [[ $UNAME == "Linux" ]]; then
#         backend="x11"
#     else
#         error "Unsupported platform." && exit 1
#     fi
# fi

# if [[ "$backend" == "cocoa" ]]; then
#     CFLAGS="$CFLAGS -DWTK_BACKEND_COCOA"
#     LFLAGS="$LFLAGS -framework Cocoa -framework OpenGL"
# elif [[ "$backend" == "x11" ]]; then
#     CFLAGS="$CFLAGS -DWTK_BACKEND_X11"
#     LFLAGS="$LFLAGS -lX11 -lGL"
# fi

# if [[ -z "$release" ]]; then
#     CFLAGS="$CFLAGS -Wextra -Werror -g3 -DDEBUG"
# else
#     CFLAGS="$CFLAGS -O3 -DNDEBUG"
# fi

# ##############################################################################

# echo -n "Compiling bin/libwtk.a with the $backend backend... "

# mkdir -p bin
# gcc $CFLAGS -c source/$backend.?
# ar crs bin/libwtk.a $backend.o
# rm $backend.o

# echo "Done."
# [[ -z "${examples}" ]] && exit 0

# for f in examples/*.c; do
#     ff=$(basename $f .c)
#     echo -n "Compiling bin/$ff... "
#     cc $CFLAGS $f -o bin/$ff $LFLAGS
#     echo "Done."
# done

# ##############################################################################