#!/usr/bin/env bash

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

while getopts ":drch" opt
do
    case "${opt}" in
        r)
            release
            ;;
        d)
            debug_symbols_on
            ;;
        c)
            clean_make
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

