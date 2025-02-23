#!/bin/sh

OS=`uname -s`
ARCH=`uname -m`
ROOT=`dirname $0`

if [ "${OS}" = "Darwin" ]; then
    REBOL=code/build/rebol/r3-mac
elif [ "${OS}" = "FreeBSD" ]; then
    REBOL=code/build/rebol/r3-freebsd
elif [ "${ARCH}" = "x86_64" ]; then
    REBOL=code/build/rebol/r3-linux64
else
    REBOL=code/build/rebol/r3-linux
fi

${ROOT}/${REBOL} -qs ${ROOT}/code/build/rebol/init.r "$@"
