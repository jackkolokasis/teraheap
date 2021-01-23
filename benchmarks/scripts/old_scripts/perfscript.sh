# !/usr/bin/env bash

###################################################
#
# file: perfscript.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  10-10-2019
# @email:    kolokasis@ics.forth.gr
#
###################################################

perf stat -e cache-references,cache-misses,major-faults,minor-faults -p $1
