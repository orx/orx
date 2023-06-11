#!/bin/bash
set -ev

cd $TRAVIS_BUILD_DIR/code/build/android
./build.sh

cd $TRAVIS_BUILD_DIR/code/demo/android
./gradlew assemble
