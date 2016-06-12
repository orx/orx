#!/bin/bash

cp ./obj/local/armeabi-v7a/liborx*.a ../../lib/static/android/armeabi-v7a
cp ./obj/local/arm64-v8a/liborx*.a ../../lib/static/android/arm64-v8a
cp ./obj/local/x86/liborx*.a ../../lib/static/android/x86

exit 0
