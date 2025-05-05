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

# Firstly : make the wb.jar
cd Whitebox
javac -sourcepath . -d . jdk/test/**/**.java
jar cf ./wb.jar .
find . -type f -name '*.class' -delete
cd ..

# Secondly : compile GC.java
make -C java GC.class

# Thirdly : compile any benchmark you want
make -C java all



