#!/bin/env python
import sys
import string

try:
    infile=file(sys.argv[1])
    outfile=file(sys.argv[2],'w')
except:
    sys.exit('Usage: python %s input_file output_file.' % sys.argv[0])



line=infile.readline()
words_now=line.lower().strip().split()
words_score=words_now[2]

while 1:

    line=infile.readline()
    if not line:
        break
    
    words_next=line.lower().strip().split()

    if (words_now[0]==words_next[0])&(words_now[1]==words_next[1]):
        while (words_now[0]==words_next[0])&(words_now[1]==words_next[1]):
            words_score=(int(words_score)+int(words_next[2]))/2
            line=infile.readline()
            words_next=line.lower().strip().split()
            if not line:
                break
        
    
    outfile.write(words_now[0]+'\t'+words_now[1]+'\t'+str(words_score)+'\n')
    words_now=words_next
    words_score=words_next[2]


