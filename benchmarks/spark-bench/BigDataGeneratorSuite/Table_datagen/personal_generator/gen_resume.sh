#!/bin/bash
##
# Function: Generate Personal Resume
#
# To Run: ./gen_resume.sh 100 10 gen_data/
#
# Output in gen_data DIR
#
# By: Mingzijian, mingzijian@ict.ac.cn
##

date

# how much parameters at least
PARA_LIMIT=2
if [ "$#" -lt $PARA_LIMIT ]; then
        echo "Usage: $0"  "NUM_RESUME" "NUMBER_OF_FILES"  "OUT_DATA_DIR"
            exit 1
        fi

##
# main part of program
##

NUM_RESUME=$1
NUMBER_OF_FILES=$2
DATA_DIR=$3
PARALLEL_NUM=21
i=1
c=1

mkdir -p "$DATA_DIR"
while [ $i -le $NUMBER_OF_FILES ]; do
    ./gen_personal_resume $NUM_RESUME $i > "$DATA_DIR/Resume_${i}"&   
        
    let i++ 
    let c++ 
    if [ $c -eq $PARALLEL_NUM ]; then
        c=1 
        wait
    fi  
done
wait

date
