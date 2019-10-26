#!/bin/bash
set -ev
cd $TRAVIS_BUILD_DIR/code/build/ios/xcode/orx-ios.xcodeproj
xcodebuild -workspace project.xcworkspace -scheme orxLIB -configuration Debug -sdk iphonesimulator -destination 'platform=iOS Simulator,name=iPad Air'
xcodebuild -workspace project.xcworkspace -scheme orxLIB -configuration Profile -sdk iphonesimulator -destination 'platform=iOS Simulator,name=iPad Air'
xcodebuild -workspace project.xcworkspace -scheme orxLIB -configuration Release -sdk iphonesimulator -destination 'platform=iOS Simulator,name=iPad Air'
cd $TRAVIS_BUILD_DIR/code/lib/static/ios
mkdir i386
mv liborx.a liborxp.a liborxd.a i386
cd $TRAVIS_BUILD_DIR/code/build/ios/xcode/orx-ios.xcodeproj
xcodebuild -workspace project.xcworkspace -scheme orxLIB -configuration Debug -sdk iphoneos
xcodebuild -workspace project.xcworkspace -scheme orxLIB -configuration Profile -sdk iphoneos
xcodebuild -workspace project.xcworkspace -scheme orxLIB -configuration Release -sdk iphoneos
cd $TRAVIS_BUILD_DIR/code/lib/static/ios
mkdir arm
mv liborx.a liborxp.a liborxd.a arm
lipo -create i386/liborxd.a arm/liborxd.a -output liborxd.a
lipo -create i386/liborxd.a arm/liborxp.a -output liborxp.a
lipo -create i386/liborxd.a arm/liborx.a -output liborx.a
cd $TRAVIS_BUILD_DIR/code/build/mac/gmake
make config=core_release64
cd $TRAVIS_BUILD_DIR/code/build/python
python package.py -p ios
