#!/usr/bin/env bash

###################################################
#
# file: conf.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-02-2021 
# @email:    kolokasis@ics.forth.gr
#
# Experiments configurations. Setup these
# configurations before run
#
###################################################
# Device for shuffle
DEV_SHFL=pmem0
# Device Fastmap
DEV_FMAP=nvme1n1
# TeraCache file size in GB e.g 800 -> 800GB
TC_FILE_SZ=900
# Executor cores
EXEC_CORES=( 16 )
# Heap size for executors '-Xms'
HEAP=( 54 )
# New generation size '-Xmn'
# if the value is 0: let the JVM to decide
# if the value > 0 : set the size of the New Generation based on the value
NEW_GEN=( 0 )
# DRAM shrink 200GB
RAMDISK=( 0 )
# Spark memory fraction: 'spark.memory.storagefraction'
MEM_FRACTON=( 0.9 )
# Storage Level
S_LEVEL=( "MEMORY_ONLY" )
# TeraCache configuration size in Spark: 'spark.teracache.heap.size'
TERACACHE=( 1200 )
# Running benchmarks
BENCHMARKS=( "LinearRegression" )
# Number of executors
EXECUTORS=1
# Total Configurations
TOTAL_CONFS=${#HEAP[@]}
