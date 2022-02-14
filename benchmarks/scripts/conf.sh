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

############## TERACACHE #######################
## Heap size for executors '-Xms'
#HEAP=( 64 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "TriangleCount" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

##############################################
## PageRank
## GC vs SD overhead
##############################################
## Heap size for executors
#HEAP=( 32 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
#RAMDISK=( 171 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "PageRank" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################

##############################################
## Connected Component
## GC vs SD overhead
##############################################
## Heap size for executors
#HEAP=( 34 136 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 204 102 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "ConnectedComponent" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################

##############################################
## Single Source Shortest Path
## GC vs SD overhead
###############################################
## Heap size for executors
#HEAP=( 102 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 136 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "ShortestPaths" )
#
### Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
##########################################

################################################
#### SVD++
#### GC vs SD overhead
################################################
## Heap size for executors
#HEAP=( 132 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 106 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "SVDPlusPlus" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################

################################################
#### TriangleCount
#### GC vs SD overhead
################################################
## Heap size for executors
#HEAP=( 220 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 0 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "TriangleCount" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################

###############################################
### Linear Regression
###############################################
## Heap size for executors
#HEAP=( 64 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 0 )
##RAMDISK=( 167 ) 
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "LinearRegression" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################

#############################################
# Logistic Regression
#############################################
# Heap size for executors
#HEAP=( 32 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 0 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "SVM" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
##########################################

###############################################
### SVM
###############################################
## Heap size for executors
#HEAP=( 35 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
### THere
#RAMDISK=( 201 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5 )
#
#S_LEVEL=( "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "SVM" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################

################################################
#### TeraCache - SVM
################################################
## Heap size for executors '-Xms'
#HEAP=( 32 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 2000 )
#
## Running benchmarks
#BENCHMARKS=( "SVM" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
#### TeraCache - Linear Regression
################################################
## Heap size for executors '-Xms'
#HEAP=( 32 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "LinearRegression" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
#### TeraCache - Linear Regression
################################################
## Heap size for executors '-Xms'
#HEAP=( 55 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 2000 )
#
## Running benchmarks
#BENCHMARKS=( "SVM" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
#### TeraCache - PageRank
################################################
## Heap size for executors '-Xms'
#HEAP=( 64 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 171 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "PageRank" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
### TeraCache - ConnectedComponent
###############################################
## Heap size for executors '-Xms'
#HEAP=( 68 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "ConnectedComponent" )
#
## Number of executors
#EXECUTORS=1

# Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
#### SSSP - PageRank
################################################
## Heap size for executors '-Xms'
#HEAP=( 42 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "ShortestPaths" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
#### TC - SVD++
################################################
## Heap size for executors '-Xms'
#HEAP=( 24 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 211 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "SVDPlusPlus" )
#
## Number of executors
#EXECUTORS=1
#
# Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

################################################
#### TC - TR
################################################
## Heap size for executors '-Xms'
#HEAP=( 64 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 171 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 900 )
#
## Running benchmarks
#BENCHMARKS=( "TriangleCount" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}


################################################
#### TC - TR
################################################
## Heap size for executors '-Xms'
#HEAP=( 32 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 0 )
#
## Spark memory fraction: 'spark.memory.storagefraction'
#MEM_FRACTON=( 0.8 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 2000 )
#
## Running benchmarks
#BENCHMARKS=( "LogisticRegression" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}


######################
# New arguments 
######################
# Device for shuffle
DEV_SHFL=pmem0

# Device Fastmap
DEV_FMAP=pmem0

# TeraCache file size in GB e.g 800 -> 800GB
TC_FILE_SZ=900

# Executor cores
EXEC_CORES=8

# Heap size for executors '-Xms'
HEAP=( 20 )

# New generation size '-Xmn'
# if the value is 0: let the JVM to decide
# if the value > 0 : set the size of the New Generation based on the value
NEW_GEN=( 0 )

# DRAM shrink 200GB
RAMDISK=( 0 )

# Spark memory fraction: 'spark.memory.storagefraction'
MEM_FRACTON=( 0.8 )

# Storage Level
S_LEVEL=( "MEMORY_ONLY" )

# TeraCache configuration size in Spark: 'spark.teracache.heap.size'
TERACACHE=( 900 )

# Running benchmarks
BENCHMARKS=( "NaiveBayes" )

# Number of executors
EXECUTORS=1

# Total Configurations
TOTAL_CONFS=${#HEAP[@]}

