#!/usr/bin/env python3

# This script is used to check if all the marked lived objects has been moved to
# their new destination place.
import sys

# Opern executor stdout
f_out = open(sys.argv[1], 'r')
lines = f_out.readlines()
check_addr = 0

for line in lines:
    line_split = line.split(' ')

    addr = int(line_split[11], 16)
    hex_addr = hex(addr)

    size = int(line_split[15])

    hex_value = hex(addr+size)

    if (check_addr == 0):
        check_addr = hex_value
        continue;
    else:
        if (check_addr != hex_addr):
            print(check_addr + " | " + hex_addr)

        check_addr = hex_value
