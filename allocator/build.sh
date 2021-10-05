#!/usr/bin/env bash

SUDO=sudo

if [ "$1" == "docker" ]
then
	SUDO=""
fi

make clean
make distclean

if [ "$1" != "docker" ]
then
	${SUDO} make uninstall
fi

make

$SUDO make install
