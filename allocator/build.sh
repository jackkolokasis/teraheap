#!/usr/bin/env bash

make clean
make distclean
sudo make uninstall
make
sudo make install
