#!/bin/bash

bakefile_gen

cd linux/autoconf

bakefilize
aclocal && autoconf

cd ..
