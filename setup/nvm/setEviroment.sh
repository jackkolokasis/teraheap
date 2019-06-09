#!/usr/bin/env bash

###################################################
#
# file: setEviroment.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-10-2018
# @email:    kolokasis@ics.forth.gr
#
# This script Build a DAX-enabled File System
#
###################################################

# Count the number of  emulated persistent memory devices we have
numEmulatedPmem=`lsblk | grep "pmem" | wc -l`

# User login
user=`whoami`

for ((i=0; i<${numEmulatedPmem}; i++))
do
    # Create an ext4 filesystem
    sudo mkfs.ext4 /dev/pmem${i}
    
    # Mount the filesystem using DAX option
    sudo mount -o dax /dev/pmem${i} /mnt/pmem${i}
    
    # Change permission owner
    sudo chown ${user}:${user} -R /mnt/pmem${i}
done
