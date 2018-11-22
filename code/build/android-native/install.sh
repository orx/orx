#!/bin/bash

cp ./obj/local/armeabi-v7a/liborx*.a ../../lib/static/android-native/armeabi-v7a
cp ./obj/local/arm64-v8a/liborx*.a ../../lib/static/android-native/arm64-v8a
cp ./obj/local/x86/liborx*.a ../../lib/static/android-native/x86
cp ./obj/local/x86_64/liborx*.a ../../lib/static/android-native/x86_64

exit 0
