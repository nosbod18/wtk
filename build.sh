#!/bin/bash

PLATFORM=$(uname -s | sed -e 's/Linux/linux/' -e 's/Darwin/macos/')
OUT=libwindow.a
SRC=src/window.$PLATFORM.?
FLAGS="-std=c99 -Wall -Wextra"

if [[ -n "$DESTDIR" ]]; then
    OUT=$DESTDIR/$OUT
fi

gcc $FLAGS -c -o $SRC.o $SRC && ar crs $OUT $SRC.o && rm $SRC.o