#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    cd $TRAVIS_BUILD_DIR/code/build/mac/gmake ;
else
  cd $TRAVIS_BUILD_DIR/code/build/linux/gmake ;
fi
make config=debug64
make config=profile64
make config=release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd $TRAVIS_BUILD_DIR/tutorial/build/mac/gmake ;
else
  cd $TRAVIS_BUILD_DIR/tutorial/build/linux/gmake ;
fi
make config=release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd $TRAVIS_BUILD_DIR/code/build/mac/gmake ;
else
  cd $TRAVIS_BUILD_DIR/code/build/linux/gmake ;
fi
make config=core_release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd $TRAVIS_BUILD_DIR/tools/orxCrypt/build/mac/gmake ;
else
  cd $TRAVIS_BUILD_DIR/tools/orxCrypt/build/linux/gmake ;
fi
make config=release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd $TRAVIS_BUILD_DIR/tools/orxFontGen/build/mac/gmake ;
else
  cd $TRAVIS_BUILD_DIR/tools/orxFontGen/build/linux/gmake ;
fi
make config=release64
cd $TRAVIS_BUILD_DIR/code/build/python
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  python package.py -p mac &&
  python package.py -p tutomac;
else
  python package.py -p linux64 &&
  python package.py -p tutolinux64;
fi