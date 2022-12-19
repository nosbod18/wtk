#!/bin/bash
cd "${0%/*}"

OS=$(uname -s | sed -e 's/Linux/linux/' -e 's/Darwin/macos/')

gcc -std=c99 -Wall -Wextra -c src/wtk.$OS.?
ar crs lib/libwtk.a *.o
rm *.o
