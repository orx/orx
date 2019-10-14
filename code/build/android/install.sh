#!/bin/bash

mkdir ../../lib/static/android/armeabi-v7a
mkdir ../../lib/static/android/arm64-v8a
mkdir ../../lib/static/android/x86
mkdir ../../lib/static/android/x86_64
cp ./obj/local/armeabi-v7a/liborx*.a ../../lib/static/android/armeabi-v7a
cp ./obj/local/arm64-v8a/liborx*.a ../../lib/static/android/arm64-v8a
cp ./obj/local/x86/liborx*.a ../../lib/static/android/x86
cp ./obj/local/x86_64/liborx*.a ../../lib/static/android/x86_64

exit 0
