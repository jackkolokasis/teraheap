#!/usr/bin/env bash

###################################################
#
# file: importNvmToSpark.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  13-10-2018
# @email:    kolokasis@ics.forth.gr
#
# This script install all the needed libraries for
# the use of emulated persistent memory. Also
# install the NVMUnsafe library for the spark
# needs
#
###################################################

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

# Make all the nvmUnsafe
cd ../nvmUnsafe/

make clean
make distclean
make all

# Check if the command executed succesfully
retValue=$?
message="Build NVMUnsafe Library" 
check ${retValue} "${message}"

make NVMUnsafe.jar

# Check if the command executed succesfully
retValue=$?
message="Build NVMUnsafe JAR file" 
check ${retValue} "${message}"

sudo make install

# Check if the command executed succesfully
retValue=$?
message="Install shared files" 
check ${retValue} "${message}"

cd -

# Install the jar file
mvn install:install-file \
    -Dfile=../nvmUnsafe/NVMUnsafe.jar \
    -DgroupId=NVMUnsafePath \
    -DartifactId=nvmUnsafe \
    -Dversion=1.0 \
    -Dpackaging=jar \
    -DpomFile=/home1/public/kolokasis/sparkPersistentMemory/spark-2.3.0/pom.xml

# Check if the command executed succesfully
retValue=$?
message="Install NVMUnsafe Library to Spark" 
check ${retValue} "${message}"
