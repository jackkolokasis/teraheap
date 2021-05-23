#! /usr/bin/python

import sys 
import string

try:
    infile=file(sys.argv[1])
    score=sys.argv[2]
    outfile=file(sys.argv[3],'w')
except:
    sys.exit('Usage: python %s input_file score output_file.' % sys.argv[0])

for line in infile:
    #print line.strip()
    #print score
    outfile.write(line.strip() + '\t' + score + '\n')

infile.close()
outfile.close()

