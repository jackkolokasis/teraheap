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

usage() {
	echo
	echo "Usage:"
	echo -n "      $0 [option ...] [-h]"
	echo
	echo "Options:"
	echo "      -r  Build release version"
	echo "      -d  Build debug version (1: FastDebug, 2: SlowDebug) "
	echo "      -c  Clean and make"
	echo "      -m  Make"
	echo "      -u  Update JVM in root directory"
	echo "      -h  Show usage"
	echo

	exit 1
}
  
# Compile without debug symbols
release() {
	make dist-clean
	CC=gcc-7.2.0 CXX=g++-7.2.0 bash ./configure --with-jobs=32 --with-target-bits=64 --with-boot-jdk="/usr/lib/jvm/java-11-openjdk" --with-extra-cflags='-O3' --with-extra-cxxflags='-O3'
	intercept-build make
	cd ../ && compdb -p openjdk11 list > compile_commands.json && mv compile_commands.json openjdk11 && cd -
	}

# Compile with debug symbols and assertions
debug_on() {
	local mode=$1
	# 1: Fast debug
	# 2: Slow debug
	make dist-clean

	if [ $mode == 1 ]
	then
		CC=gcc-7.2.0 CXX=g++-7.2.0 bash ./configure --with-jobs=32 --with-target-bits=64 --enable-asan --enable-debug --with-native-debug-symbols=internal
	elif [ $mode == 2 ]
	then
		CC=gcc-7.2.0 CXX=g++-7.2.0 bash ./configure --with-jobs=32 --with-target-bits=64 --enable-asan --with-debug-level=slowdebug --with-native-debug-symbols=internal
	fi

	intercept-build make
	cd ../ && compdb -p openjdk11 list > compile_commands.json && mv compile_commands.json openjdk11 && cd -
}

clean_make() {
	make clean
	make
}

update() {
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

export_env_vars() {
	local PROJECT_DIR="$(pwd)/../.."

	export JAVA_HOME="/usr/lib/jvm/java-1.8.0"
}

#### --with-extra-ldflags 

while getopts ":d:rcmhu" opt
do
    case "${opt}" in
        r)
			export_env_vars
            release
            ;;
        d)
			MODE=${OPTARG}
			export_env_vars
            debug_on ${MODE}
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

