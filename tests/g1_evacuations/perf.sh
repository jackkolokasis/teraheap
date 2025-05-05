#!/usr/bin/env bash

pid=$(jps | grep HashMap | awk '{print $1}')
perf stat -o perf.out -e cache-references,cache-misses,page-faults,major-faults,minor-faults,dTLB-load-misses,dTLB-store-misses -p ${pid}
grep "Misses" ./java/out/gc_log.log | awk '{ sum += $NF } END { print "Sum of cache misses:", sum }'