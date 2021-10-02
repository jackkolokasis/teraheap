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

############## SVM #######################
# D = 196, E = 4, S = 64

# Heap size for executors
#HEAP=( 4 24 44 83 122 161 200 )

# DRAM shrink
#RAMDISK=( 226 206 186 147 108 69 30 )

# Spark memory fraction
#MEM_FRACTON=( 0.2 0.83 0.9 0.95 0.97 0.98 0.98 )
############################################

############## KMEANS #######################
# Heap size for executors
# D = 30, E = 4, S = 64
#HEAP=( 4 7 10 16 22 28 32 )

# DRAM shrink
#RAMDISK=( 226 223 220 214 208 202 198 )

# Spark memory fraction
#MEM_FRACTON=( 0.2 0.4 0.6 0.75 0.82 0.86 0.88 )
############################################

############### LgR #######################
## Heap size for executors
## D = 160, E = 4, S = 64
##HEAP=( 4 20 36 68 100 132 164 )
#HEAP=( 12 )
#
### DRAM shrink
##RAMDISK=( 226 210 194 162 130 98 66 )
#RAMDISK=( 0 )
#
#NEW_GEN=( 0 )
#
### Spark memory fraction
##MEM_FRACTON=( 0.2 0.8 0.89 0.94 0.96 0.97 0.98 )
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
#############################################

############## LR #######################
# Heap size for executors
# D = 160, E = 4, S = 64
# HEAP=( 4 20 36 68 100 132 164 )

# DRAM shrink
# RAMDISK=( 226 210 194 162 130 98 66 )

# Spark memory fraction
# MEM_FRACTON=( 0.2 0.8 0.89 0.9 0.96 0.97 0.98 )
############################################

############## KMeans #######################
# Heap size for executors
# D = 60, E = 8, S = 128
# HEAP=( 8 14 20 32 44 56 68)

# DRAM shrink
# RAMDISK=( 222 216 210 198 186 174 162 )

# Spark memory fraction
# MEM_FRACTON=( 0.2 0.43 0.6 0.75 0.82 0.86 0.9 )
############################################

############### SVM #######################
### D = 196, E = 4, S = 64 f = E/H
#
### Heap size for executors
#HEAP=( 4 24 44 83 122 161 )
#
### DRAM shrink
#RAMDISK=( 230 212 190 147 112 73 )
#
### Spark memory fraction
#MEM_FRACTON=( 0 0 0 0 0 0 )
#
#S_LEVEL=( "DISK_ONLY" "MEMORY_AND_DISK" "MEMORY_AND_DISK" "MEMORY_AND_DISK" \
#	"MEMORY_AND_DISK" "MEMORY_AND_DISK" )
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



############## TERACACHE #######################
## Heap size for executors '-Xms'
#HEAP=( 8 )
#
## New generation size '-Xmn'
## if the value is 0: let the JVM to decide
## if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
## DRAM shrink 200GB
#RAMDISK=( 235 )
#
## Spark memory fraction: 'spark.memory.fraction'
#MEM_FRACTON=( 0.5 )
#
## Storage Level
#S_LEVEL=( "MEMORY_ONLY" )
#
## TeraCache configuration size in Spark: 'spark.teracache.heap.size'
#TERACACHE=( 220 )
#
## Running benchmarks
#BENCHMARKS=( "KMeans" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}


############### LgR #######################
## Heap size for executors
## D = 160, E = 4, S = 64
#HEAP=( 4 20 36 68 100 132 )
#
## DRAM shrink
#RAMDISK=( 226 210 194 162 130 98 )
#
## Spark memory fraction
#MEM_FRACTON=( 0.5 0.5 0.5 0.5 0.5 0.5 )
#
#S_LEVEL=( "DISK_ONLY" "MEMORY_AND_DISK" "MEMORY_AND_DISK" "MEMORY_AND_DISK" \
#	"MEMORY_AND_DISK" "MEMORY_AND_DISK" )
#
### Running benchmarks
#BENCHMARKS=( "LogisticRegression" )
#
### Number of executors
#EXECUTORS=1
#
### Total Configurations
#TOTAL_CONFS=${#HEAP[@]}
###########################################


############### LgR #######################
## Heap size for executors
## D = 160, E = 4, S = 64
#HEAP=( 20 )
#
## DRAM shrink
#RAMDISK=( 0 )
#
## Spark memory fraction
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


############### LR #######################
#
### Heap size for executors
#HEAP=( 16 )
#
### New generation size '-Xmn'
### if the value is 0: let the JVM to decide
### if the value > 0 : set the size of the New Generation based on the value
#NEW_GEN=( 0 )
#
### DRAM shrink
#RAMDISK=( 227 )
#
### Spark memory fraction
#MEM_FRACTON=( 0.5)
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

############## TERACACHE #######################
# Heap size for executors '-Xms'
HEAP=( 32 )

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
BENCHMARKS=( "LinearRegression" )

# Number of executors
EXECUTORS=1

# Total Configurations
TOTAL_CONFS=${#HEAP[@]}

#############VANILLA######################
#Heap size for executors '-Xms'
#HEAP=( 35 )
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
#MEM_FRACTON=( 0.5 )
#
## Storage Level
#S_LEVEL=( "MEMORY_AND_DISK" )
#
## Running benchmarks
#BENCHMARKS=( "SVM" )
#
## Number of executors
#EXECUTORS=1
#
## Total Configurations
#TOTAL_CONFS=${#HEAP[@]}

