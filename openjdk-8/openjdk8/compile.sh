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
  CC=gcc-4.8.5 CXX=g++-4.8.5 bash ./configure --with-jobs=32 --disable-debug-symbols --with-extra-cflags='-O3' --with-extra-cxxflags='-O3'
  intercept-build make
  cd ../ && compdb -p openjdk8 list > compile_commands.json && mv compile_commands.json openjdk8 && cd -
}

# Compile with debug symbols and assertions
function debug_symbols_on() 
{
  make dist-clean
  CC=gcc-4.8.5 CXX=g++-4.8.5 bash ./configure --with-debug-level=release --with-target-bits=64 --disable-zip-debug-info --with-jobs=32
  intercept-build make
  cd ../ && compdb -p openjdk8 list > compile_commands.json && mv compile_commands.json openjdk8 && cd -
}

function clean_make()
{
  make clean
  make
}

function update()
{
	export_env_vars
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

export_env_vars()
{
	local PROJECT_DIR="$(pwd)/../.."

	export JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk"

	### TeraHeap Allocator
	export LIBRARY_PATH=${PROJECT_DIR}/allocator/lib/:$LIBRARY_PATH
	export LD_LIBRARY_PATH=${PROJECT_DIR}/allocator/lib/:$LD_LIBRARY_PATH                                                                                           
	export PATH=${PROJECT_DIR}/allocator/include/:$PATH
	export C_INCLUDE_PATH=${PROJECT_DIR}/allocator/include/:$C_INCLUDE_PATH                                                                                         
	export CPLUS_INCLUDE_PATH=${PROJECT_DIR}/allocator/include/:$CPLUS_INCLUDE_PATH

	### CThread Pool Library
	export LIBRARY_PATH=${PROJECT_DIR}/C-Thread-Pool/lib/:$LIBRARY_PATH
	export LD_LIBRARY_PATH=${PROJECT_DIR}/C-Thread-Pool/lib/:$LD_LIBRARY_PATH
	export PATH=${PROJECT_DIR}/C-Thread-Pool/include/:$PATH
	export C_INCLUDE_PATH=${PROJECT_DIR}/C-Thread-Pool/include/:$C_INCLUDE_PATH
	export CPLUS_INCLUDE_PATH=${PROJECT_DIR}/C-Thread-Pool/include/:$CPLUS_INCLUDE_PATH
}

while getopts ":drcmhu" opt
do
    case "${opt}" in
        r)
			export_env_vars
            release
            ;;
        d)
			export_env_vars
            debug_symbols_on
            ;;
        c)
			echo "Clean and make"
			export_env_vars
            clean_make
            ;;
        u)
			echo "Update JVM in root directory"
			update
			;;
        m)
			echo "Make"
			export_env_vars
			make
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

