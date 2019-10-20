#!/bin/bash
set -ev

cd $TRAVIS_BUILD_DIR/code/build/android-native
$ANDROID_NDK_HOME/ndk-build
./install.sh
cd $TRAVIS_BUILD_DIR/code/demo/android-native
./gradlew assemble

cd $TRAVIS_BUILD_DIR/code/build/android
$ANDROID_NDK_HOME/ndk-build
./install.sh
cd $TRAVIS_BUILD_DIR/code/demo/android
./gradlew assemble