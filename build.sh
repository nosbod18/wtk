#!/bin/bash
cd "${0%/*}"
OS=$(uname -s | sed -e 's/Linux/linux/' -e 's/Darwin/macos/')

#######################################

NAME=window
SRCS=(src/window.$OS.?)
DEPS=()
LIBS=()
FLGS=(-std=c99 -Wall -Wextra)

#######################################

function bin() {
    gcc ${FLGS[@]} -o bin/$NAME ${SRCS[@]} ${LIBS[@]}
}

function lib() {
    gcc ${FLGS[@]} -c ${SRCS[@]}
    ar crs lib/lib$NAME.a *.o
    rm *.o
}

#######################################

for d in ${DEPS[@]}; do
    ./$d/build.sh lib
    LIBS=(-L$d -l$(basename $d) ${LIBS[@]})
done

case $1 in
    lib) mkdir -p $1 && lib;;
    bin) mkdir -p $1 && bin;;
    *)   echo "Usage $0 <bin|lib>" && exit 1;;
esac

#######################################