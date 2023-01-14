#!/bin/bash
SRCS="src/wtk.c main.c"
FLGS="-Wall -Wextra -Wno-deprecated-declarations -std=c99 -x objective-c"
LIBS="-framework Cocoa -framework OpenGL"

gcc $FLGS $SRCS $LIBS && ./a.out