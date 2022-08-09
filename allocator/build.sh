#!/usr/bin/env bash

enviroment() {
### TeraCache Allocator
export LIBRARY_PATH=/home1/public/kolokasis/github/teracache/allocator/lib/:$LIBRARY_PATH
export LD_LIBRARY_PATH=/home1/public/kolokasis/github/teracache/allocator/lib/:$LD_LIBRARY_PATH
export PATH=/home1/public/kolokasis/github/teracache/allocator/include/:$PATH
export C_INCLUDE_PATH=/home1/public/kolokasis/github/teracache/allocator/include/:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=/home1/public/kolokasis/github/teracache/allocator/include/:$CPLUS_INCLUDE_PATH
}

make clean
make distclean
make
