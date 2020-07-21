#!/usr/bin/env bash


#make clean
#bash ./configure
#intercept-build make

bash ./configure --with-debug-level=release --with-target-bits=64 --disable-zip-debug-info --with-jobs=16
#intercept-build make
make
