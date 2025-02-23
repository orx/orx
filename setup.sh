#!/bin/sh

OS=`uname -s`
ARCH=`uname -m`

if [ "${OS}" = "Darwin" ]; then
    REBOL=code/build/rebol/r3-mac
elif [ "${OS}" = "FreeBSD" ]; then
    REBOL=code/build/rebol/r3-freebsd
elif [ "${ARCH}" = "x86_64" ]; then
    REBOL=code/build/rebol/r3-linux64
else
    REBOL=code/build/rebol/r3-linux
fi

${REBOL} -qs code/build/rebol/setup.r "$@"
