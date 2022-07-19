
#!/usr/bin/env bash
###################################################
#
# file: build.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  22-10-2021
# @email:    kolokasis@ics.forth.gr
#
# This script install all the needed libraries for
# the use of emulated persistent memory.
###################################################

COMPILE_OUT=$1
SPARK_DIR=$2

# Check if the last command executed succesfully
#
# if executed succesfully, print SUCCEED
# if executed with failures, print FAIL and exit
check () {
    if [ $1 -ne 0 ]
    then
        echo
        echo -e "  $2 \e[40G [\e[31;1mFAIL\e[0m]"
        echo
        exit
    else
        echo
        echo -e "  $2 \e[40G [\e[32;1mSUCCED\e[0m]"
        echo
    fi
}

check_dependencies() {
  local libvemem_so=$(whereis -b libvmem | awk -F ':' '{print $2}' | grep ".so")
  local libvemem_h=$(whereis -b libvmem | awk -F ':' '{print $2}' | grep ".h")

  if [[ ! -n ${libvmem_so} ]]
  then
    sudo yum install libvmem >> ${COMPILE_OUT} 2>&1

    # Check if the command executed succesfully
    retValue=$?
    message="Install libvmem library" 
    check ${retValue} "${message}"
  fi

  if [[ ! -n ${libvmem_h} ]]
  then
    sudo yum install libvmem-devel.x86_64 >> ${COMPILE_OUT} 2>&1
    
    # Check if the command executed succesfully
    retValue=$?
    message="Install libvmem header files" 
    check ${retValue} "${message}"
  fi
}
make clean >> ${COMPILE_OUT} 2>&1
make distclean >> ${COMPILE_OUT} 2>&1
make all >> ${COMPILE_OUT} 2>&1

# Check if the command executed succesfully
retValue=$?
message="Build NVMUnsafe Library" 
check ${retValue} "${message}"

make NVMUnsafe.jar >> ${COMPILE_OUT} 2>&1

# Check if the command executed succesfully
retValue=$?
message="Build NVMUnsafe JAR file" 
check ${retValue} "${message}"

# Install the jar file
mvn install:install-file \
    -Dfile=NVMUnsafe.jar \
    -DgroupId=NVMUnsafePath \
    -DartifactId=nvmUnsafe \
    -Dversion=1.0 \
    -Dpackaging=jar \
    -DpomFile="${SPARK_DIR}/pom.xml" >> ${COMPILE_OUT} 2>&1

# Check if the command executed succesfully
retValue=$?
message="Install NVMUnsafe Library to Spark" 
check ${retValue} "${message}"
