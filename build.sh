#!/bin/bash

CFLAGS="-std=c99 -Wall -Wextra -Werror -Iinclude -Isource -g3"
LFLAGS="-Lbin -lwtk"
UNAME="$(uname -s)"

if [[ $UNAME == "Darwin" ]]; then
    CFLAGS="$CFLAGS -DWTK_USE_COCOA -x objective-c"
    LFLAGS="$LFLAGS -framework Cocoa -framework OpenGL"
elif [[ $UNAME == "Linux" ]]; then
    CFLAGS="$CFLAGS -DWTK_USE_X11"
    LFLAGS="$LFLAGS -lX11 -lGL"
else
    printf "\033[1;31mError: Unsupported platform\033[0m\n"
fi

echo -n "Compiling bin/libwtk.a... "

mkdir -p bin
gcc $CFLAGS -c source/wtk.c -o bin/wtk.o
ar crs bin/libwtk.a bin/wtk.o
rm bin/wtk.o

echo "Done"

echo -n "Compiling bin/01-simple... "   && gcc $CFLAGS examples/01-simple.c   -o bin/01-simple   $LFLAGS && echo "Done"
echo -n "Compiling bin/02-events... "   && gcc $CFLAGS examples/02-events.c   -o bin/02-events   $LFLAGS && echo "Done"
echo -n "Compiling bin/03-triangle... " && gcc $CFLAGS examples/03-triangle.c -o bin/03-triangle $LFLAGS && echo "Done"
