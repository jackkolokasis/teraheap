#!/usr/bin/env python3
import sys, os, datetime
import dateutil.parser as dp
# true if string is a positive float
def validSeconds(str_sec):
    try:
        return 0 < float(str_sec)
    except ValueError:
        return False
                
# show usage                
if len(sys.argv) < 2:
    print "Usage: %s " % (sys.argv[0])
    sys.exit(1)
    
file_str = sys.argv[1]

lastmod_date = datetime.datetime.fromtimestamp(os.path.getmtime(file_str))

file = open(file_str, 'r')
lines = file.readlines()
file.close()

# get last elapsed time
for line in reversed(lines):
    parts = line.split(':')
    if "2019" in line: 
        break

# calculate start time
start_date = lastmod_date - datetime.timedelta(seconds=float(parts[3]))
  
# print file prepending human readable time where appropiate  
for line in lines:
    parts = line.split(':')
    if "2019" not in line: 
        continue
    if "secs" in line:
        line_date = start_date + datetime.timedelta(seconds=float(parts[3]))
        print "%s: %s" % (line_date.isoformat(), line.rstrip())
