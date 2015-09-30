#!/bin/sh

OS=`uname -s`

if [ "${OS}" = "Darwin" ]; then
    REBOL=code/build/rebol/r3-mac
else
    REBOL=code/build/rebol/r3-linux
fi

${REBOL} code/build/rebol/setup.r
