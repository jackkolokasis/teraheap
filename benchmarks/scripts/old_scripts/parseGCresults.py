#!/usr/bin/env python
import sys, os, datetime

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
    if "2019-09-19T" not in parts[0]:
        continue

    if validSeconds(parts[3]):
        break

# calculate start time
start_date = lastmod_date - datetime.timedelta(seconds=float(parts[3]))
  
# print file prepending human readable time where appropiate  
for line in lines:
    parts = line.split(':')
    if "2019-09-19T" not in parts[0]:
        continue
    if "Application time" in parts[4]:
        continue
    if "Total time for which" in parts[4]:
        continue

    if not validSeconds(parts[3]):
        print line.rstrip()
        continue
    line_date = start_date + datetime.timedelta(seconds=float(parts[3]))
    print "%s: %s" % (line_date.isoformat(), line.rstrip())


"""
# GC log file parse
file_str = sys.argv[1]

file = open(file_str, 'r')
lines = file.readlines()
file.close()

user = 0
sys = 0
real = 0

for line in lines:
    parts = line.split(" ")
    
    user = user + float(parts[2].split("=")[1])
    sys  = sys  + float(parts[3].split("=")[1].replace(',',''))
    real = real + float(parts[4].split("=")[1])
   
print "User Time = " + str(user)
print "Sys Time = "  + str(sys)
print "Real Time = " + str(real)
"""
"""
file_str = sys.argv[1]
# CPU log file parse
file = open(file_str, 'r')
lines = file.readlines()
file.close()

user = 0
sys = 0
iowait = 0
idle = 0

for line in lines:
    parts = line.split(";")
    
    user = user + float(parts[2])
    sys  = sys  + float(parts[4])
    iowait = iowait + float(parts[6])
    idle = idle + float(parts[8])
   
print "User Time = " + str(user)
print "Sys Time = "  + str(sys)
print "IOwait Time = " + str(iowait)
print "Idle Time = " + str(idle)
"""
# GC log file parse
file_str = sys.argv[1]

file = open(file_str, 'r')
lines = file.readlines()
file.close()

user = 0
sys = 0
real = 0

for line in lines:
    parts = line.split(" ")
    
    user = user + float(parts[2].split("=")[1])
    sys  = sys  + float(parts[3].split("=")[1].replace(',',''))
    real = real + float(parts[4].split("=")[1])
   
print "User Time = " + str(user)
print "Sys Time = "  + str(sys)
print "Real Time = " + str(real)

"""
file_str = sys.argv[1]

file = open(file_str, 'r')
lines = file.readlines()
file.close()

total = []
avg   = []

for line in lines:
    parts = line.split(" ")

    avg.append(float(parts[12].replace(',', '')))
    total.append(float(parts[18].replace(']', '')))

    total_sum = sum(total)
    total_avg = sum(avg)

    max_total = max(total)
    max_avg   = max(avg)

    min_total = min(total)
    min_avg   = min(avg)

print "Sum(Total) = " + str(total_sum)
print "Sum(Avg) = "  + str(total_avg)

print "Max(Total) = " + str(max_total)
print "Max(Avg) = " + str(max_avg)

print "Min(Total) = " + str(min_total)
print "Min(Avg) = " + str(min_avg)
"""

"""
file_str = sys.argv[1]

file = open(file_str, 'r')
lines = file.readlines()
file.close()

total = []
avg   = []

for line in lines:
    parts = line.split(" ")

    print parts[12].replace(',', '')
    print parts[18].replace(',', '')

    avg.append(float(parts[12].replace(',', '')))
    total.append(float(parts[18].replace(']', '')))

    total_sum = sum(total)
    total_avg = sum(avg)

    max_total = max(total)
    max_avg   = max(avg)

    min_total = min(total)
    min_avg   = min(avg)

print "Sum(Total) = " + str(total_sum)
print "Sum(Avg) = "  + str(total_avg)

print "Max(Total) = " + str(max_total)
print "Max(Avg) = " + str(max_avg)

print "Min(Total) = " + str(min_total)
print "Min(Avg) = " + str(min_avg)
"""
