#!/usr/bin/env bash

###################################################
#
# file: compile.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  07-03-2021 
# @email:    kolokasis@ics.forth.gr
#
# Compile JVM
#
###################################################

# Define some variables for pretty printing
ESC='\033[' 

# Attributes
NORMAL=0
BOLD=1
UNDERLINE=4
BLINK=5
REVERSE_V=7
INVISIBLE=8

# Foreground colors
BLACK_FG=30
RED_FG=31
GREEN_FG=32
YELLOW_FG=33
BLUE_FG=34
MAGENTA_FG=35
CYAN_FG=36
WHITE_FG=37

# Background colors
BLACK_BG=40
RED_BG=41
GREEN_BG=42
YELLOW_BG=43
BLUE_BG=44
MAGENTA_BG=45
CYAN_BG=46
WHITE_BG=47

# Presets
BRED=${ESC}${BOLD}';'${RED_FG}'m'
BGREEN=${ESC}${BOLD}';'${GREEN_FG}'m'
BYELLOW=${ESC}${BOLD}';'${YELLOW_FG}'m'
BBLUE=${ESC}${BOLD}';'${BLUE_FG}'m'
BMAGENTA=$ESC$BOLD';'$MAGENTA_FG'm'
BCYAN=$ESC$BOLD';'$CYAN_FG'm'
BWHITE=$ESC$BOLD';'$WHITE_BG'm'
RESET=${ESC}${NORMAL}'m'

function usage()
{
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-h]"
    echo
    echo "Options:"
    echo "      -r  Build release without debug symbols"
    echo "      -d  Build with debug symbols"
    echo "      -c  Clean and make"
    echo "      -u  Update JVM in root directory"
    echo "      -h  Show usage"
    echo

    exit 1
}
  
# Compile without debug symbols
function release() 
{
  make dist-clean
  bash ./configure --with-jobs=32 --disable-debug-symbols --with-extra-cflags='-O3' --with-extra-cxxflags='-O3'
  #intercept-build make
  #cd ../ && compdb -p openjdk8 list > compile_commands.json && mv compile_commands.json openjdk8 && cd -
}

# Compile with debug symbols and assertions
function debug_symbols_on() 
{
  make dist-clean
  bash ./configure --with-debug-level=release --with-target-bits=64 --disable-zip-debug-info --with-jobs=32
  #bash ./configure --with-debug-level=slowdebug --with-target-bits=64 --disable-zip-debug-info --with-jobs=32
  #intercept-build make
  #cd ../ && compdb -p openjdk8 list > compile_commands.json && mv compile_commands.json openjdk8 && cd -
}

function clean_make()
{
  make clean
  make
}

function update()
{
	make

	if [ $? -ne 0 ]; then
		echo "[${BRED}COMPILE${ESC}] Compilation failed"
		exit
	fi

	clear
	echo -e "[${BGREEN}COMPILE${RESET}] JVM"

	cd /usr/lib/jvm/java-8-kolokasis/build
	sudo rm -rf linux-x86_64-normal-server-release
	cd - > /dev/null
	sudo cp -r build/linux-x86_64-normal-server-release /usr/lib/jvm/java-8-kolokasis/build

	echo -e "[${BYELLOW}INSTALL${RESET}] JVM"
}

while getopts ":drchu" opt
do
    case "${opt}" in
        r)
            release
            ;;
        d)
            debug_symbols_on
            ;;
        c)
			echo "Clean and make"
            clean_make
            ;;
        u)
			echo "Update JVM in root directory"
			update
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

