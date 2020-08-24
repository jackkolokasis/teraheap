#!/usr/bin/env python3

# This script is used to check if all the marked lived objects has been moved to
# their new destination place.
import sys

# Opern executor stdout
f_out = open(sys.argv[1], 'r')
lines = f_out.readlines()

precompact_dict = {}
memmove_dict = {}

for line in lines:
    if "PRECOMPACT" in line:
        line_split = line.split(' ')
        precompact_dict[line_split[4]] = 1

# Open executor stderr
f_err = open(sys.argv[2], 'r')
lines = f_err.readlines()

for line in lines:
    if "MEMMOVE" in line:
        line_split = line.split(' ')
        old_addr = str(line_split[4])
        new_addr = str(line_split[8])

        memmove_dict[old_addr] = 1

for k in precompact_dict.keys():
    if k not in memmove_dict.keys():
        print("%s" % (k))



