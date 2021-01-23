#!/usr/bin/env python3

# This script is used to check if all the marked lived objects has been moved to
# their new destination place.
import sys

# Opern executor stdout
f_out = open(sys.argv[1], 'r')
lines = f_out.readlines()

prev_end = 0
prev_obj = 0

end = 0
obj = 0

for line in lines:
    line_split = line.split(' ')
    if (prev_end == 0):
        end = line_split[12].rstrip("\n")
        obj= line_split[4]
        
        prev_end = end.rstrip("\n")
        prev_obj= obj

    else:
        end = line_split[12].rstrip("\n")
        obj= line_split[4]

        if obj not in prev_end:
            print("%s -> %s" % (prev_end, obj))

        prev_end = end
        prev_obj= obj
