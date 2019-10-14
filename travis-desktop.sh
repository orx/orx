#!/bin/bash
set -ev
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    cd ~/build/orx/orx/code/build/mac/gmake ;
else
  cd ~/build/orx/orx/code/build/linux/gmake ;
fi
make config=debug64
make config=profile64
make config=release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd ~/build/orx/orx/tutorial/build/mac/gmake ;
else
  cd ~/build/orx/orx/tutorial/build/linux/gmake ;
fi
make config=release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd ~/build/orx/orx/code/build/mac/gmake ;
else
  cd ~/build/orx/orx/code/build/linux/gmake ;
fi
make config=core_release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd ~/build/orx/orx/tools/orxCrypt/build/mac/gmake ;
else
  cd ~/build/orx/orx/tools/orxCrypt/build/linux/gmake ;
fi
make config=release64
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cd ~/build/orx/orx/tools/orxFontGen/build/mac/gmake ;
else
  cd ~/build/orx/orx/tools/orxFontGen/build/linux/gmake ;
fi
make config=release64
cd ~/build/orx/orx/code/build/python
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  python package.py -p mac &&
  python package.py -p tutomac;
else
  python package.py -p linux64 &&
  python package.py -p tutolinux64;
fi