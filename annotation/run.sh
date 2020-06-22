#!/usr/bin/env bash

make clean
make distclean
sudo make uninstall
make all
make TeraCache.jar
sudo make install
