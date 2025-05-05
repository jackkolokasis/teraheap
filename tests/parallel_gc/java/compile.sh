#!/usr/bin/env bash

PROJECT_DIR="$(pwd)/../.."

export LIBRARY_PATH=${PROJECT_DIR}/allocator/lib/:$LIBRARY_PATH
export LD_LIBRARY_PATH=${PROJECT_DIR}/allocator/lib/:$LD_LIBRARY_PATH
export PATH=${PROJECT_DIR}/allocator/include/:$PATH
export C_INCLUDE_PATH=${PROJECT_DIR}/allocator/include/:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=${PROJECT_DIR}/allocator/include/:$CPLUS_INCLUDE_PATH

export LIBRARY_PATH=${PROJECT_DIR}/tera_malloc/lib/:$LIBRARY_PATH
export LD_LIBRARY_PATH=${PROJECT_DIR}/tera_malloc/lib/:$LD_LIBRARY_PATH
export PATH=${PROJECT_DIR}/tera_malloc/include/:$PATH
export C_INCLUDE_PATH=${PROJECT_DIR}/tera_malloc/include/:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=${PROJECT_DIR}/tera_malloc/include/:$CPLUS_INCLUDE_PATH

# Usage function to display script usage
usage() {
  echo "Usage: $0 -r | -o | -d"
  echo "  -r    Compile in release mode"
  echo "  -o    Compile in optimized mode(release mode with debug sumbols)"
  echo "  -d    Compile in fastdebug mode"
  exit 1
}

# Check if an argument is provided
if [ $# -ne 1 ]; then
  usage
fi

# Parse the argument
while getopts "rod" opt; do
  case ${opt} in
    r)
      MODE="release"
      ;;
    o)
      MODE="optimized"
      ;;
    d)
      MODE="debug"
      ;;
    *)
      usage
      ;;
  esac
done

# Execute based on the mode
if [ "$MODE" == "release" ]; then
  echo "Compile with release mode..."
  make -f Makefile_release all
elif [ "$MODE" == "optimized" ]; then
  echo "Compile in optimized  mode..."
  make -f Makefile_release all
elif [ "$MODE" == "debug" ]; then
  echo "Compile in fastdebug mode..."
  make -f Makefile_fastdebug all
else
  usage
fi
