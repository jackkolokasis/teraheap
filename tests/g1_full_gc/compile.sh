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

# Compile any benchmark you want
# make -C phases Phase1.java

# Or compile all of them:
make -C java
make -C phases


