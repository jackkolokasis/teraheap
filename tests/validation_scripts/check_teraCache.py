#!/usr/bin/env python3

# This script is used to check if all the cached data objects has been moved to
# TeraCache succesfully
import sys

# Opern executor stdout
f_out = open(sys.argv[1], 'r')
lines = f_out.readlines()

tera_cache_dict = {}
memmove_dict = {}

for line in lines:
    if "TC_REGION_TOP" in line:
        line_split = line.split(' ')
        tera_cache_dict[line_split[5]] = line_split[9]

# Open executor stderr
f_err = open(sys.argv[2], 'r')
lines = f_err.readlines()

for line in lines:
    if "MEMMOVE" in line:
        line_split = line.split(' ')
        old_addr = str(line_split[4])
        new_addr = str(line_split[8])

        if old_addr in tera_cache_dict:
            if (str(tera_cache_dict.get(old_addr)) == new_addr):
                memmove_dict[old_addr] = new_addr

for k in tera_cache_dict.keys():
    if k not in memmove_dict:
        print("%s ==> %s" % (k, tera_cache_dict[k]))



