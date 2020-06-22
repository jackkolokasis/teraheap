#!/usr/bin/env bash

###################################################
#
# file: importNvmToSpark.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  13-10-2018
# @Version:  21-06-2020
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
        echo -e "  $2 \e[40G [\e[31;1mFAIL\e[0m]"
        exit
    else
        echo -e "  $2 \e[40G [\e[32;1mSUCCED\e[0m]"
    fi
}

make clean >/dev/null 2>&1
make distclean >/dev/null 2>&1
sudo make uninstall >/dev/null 2>&1
make all >/dev/null 2>&1

# Check if the command executed succesfully
retValue=$?
message="Build TeraCache Library" 
check ${retValue} "${message}"

make TeraCache.jar >/dev/null 2>&1

# Check if the command executed succesfully
retValue=$?
message="Build TeraCache JAR file" 
check ${retValue} "${message}"

sudo make install >/dev/null 2>&1

# Check if the command executed succesfully
retValue=$?
message="Install shared files" 
check ${retValue} "${message}"

# Install the jar file
mvn install:install-file \
    -Dfile=TeraCache.jar \
    -DgroupId=TeraCachePath \
    -DartifactId=teraCache \
    -Dversion=1.0 \
    -Dpackaging=jar \
    -DpomFile=/opt/spark/spark-2.3.0-kolokasis/pom.xml >/dev/null 2>&1

# Check if the command executed succesfully
retValue=$?
message="Install TeraCache Library to Spark" 
check ${retValue} "${message}"
