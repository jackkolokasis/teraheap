#!/usr/bin/env python3
import re
import sys
import os

file_str = sys.argv[1]

file = open(file_str, 'r')
lines = file.readlines()

sum = 0

for line in lines:
    if "%" in line:
        regex = re.findall("\\d+(?:\\.\\d+)?%", line)
        sum = sum + float(regex[0].split('%')[0])

# Output file
outputFile = open(sys.argv[2], 'a')

# Write the results to the output file


if mode == 0:
    print("Ser;" + str(sum))
else:
    print("Deser;" + str(sum))

