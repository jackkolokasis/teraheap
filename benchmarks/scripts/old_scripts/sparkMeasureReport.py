#!/usr/bin/env python

###################################################
#
# file: sparkMeasureReport.py
#
# @Author:   Iacovos G. Kolokasis
# @Version:  18-03-2019
# @email:    kolokasis@ics.forth.gr
#
# Spark Meseaure Report
#
###################################################


import sys, getopt
import operator
import optparse
from pyspark.context import SparkContext
from pyspark.sql.session import SparkSession

# Parse input arguments
usage = "usage: %prog [options]"
parser = optparse.OptionParser(usage=usage)
parser.add_option("-o", "--output", dest="output", metavar="FILE", help="Output file")
(options, args) = parser.parse_args()

# This is the json file path and name where the metrics are stored
metrics_filename = "/home1/public/kolokasis/sparkPersistentMemory/benchmarks/scripts/taskMetrics.serialized"

begin_time = 0                               # Start Time
end_time = 0                                 # End Time

sc = SparkContext('local')
spark = SparkSession(sc)

df = spark.read.json(metrics_filename, multiLine=True)

from pyspark.sql import functions as F

if (end_time == 0):
    end_time = df.agg(F.max(df.finishTime)).collect()[0][0]    
    
if (begin_time == 0):
    begin_time = df.agg(F.min(df.launchTime)).collect()[0][0]    

df.filter("launchTime >= {0} and finishTime <= {1}"\
 .format(begin_time, end_time))\
 .createOrReplaceTempView("PerfTaskMetrics")

report = spark.sql("""select count(*) numtasks, max(finishTime) - min(launchTime) as elapsedTime, sum(duration), sum(schedulerDelay),
       sum(executorRunTime), sum(executorCpuTime), sum(executorDeserializeTime), sum(executorDeserializeCpuTime),
       sum(resultSerializationTime), sum(jvmGCTime), sum(shuffleFetchWaitTime), sum(shuffleWriteTime), sum(gettingResultTime),
       max(resultSize), sum(numUpdatedBlockStatuses), sum(diskBytesSpilled), sum(memoryBytesSpilled), 
       max(peakExecutionMemory), sum(recordsRead), sum(bytesRead), sum(recordsWritten), sum(bytesWritten),
       sum(shuffleTotalBytesRead), sum(shuffleTotalBlocksFetched), sum(shuffleLocalBlocksFetched), 
       sum(shuffleRemoteBlocksFetched), sum(shuffleBytesWritten), sum(shuffleRecordsWritten) 
       from PerfTaskMetrics""").toPandas().transpose()

report.columns=['Metric Value']

report.to_csv('%s' % options.output)
