#!/bin/sh

OS=`uname -s`
ARCH=`uname -m`
ROOT=`dirname $0`

if [ "${OS}" = "Darwin" ]; then
    REBOL=code/build/rebol/r3-mac
else
    if [ "${ARCH}" = "x86_64" ]; then
        REBOL=code/build/rebol/r3-linux64
    else
        REBOL=code/build/rebol/r3-linux
    fi
fi

${ROOT}/${REBOL} ${ROOT}/code/build/rebol/init.r "$@"
