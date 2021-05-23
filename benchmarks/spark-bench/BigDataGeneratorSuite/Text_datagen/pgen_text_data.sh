#!/bin/sh

##
# Function: Generate text data
#
# To Run: pgen_text_data.sh lda_wiki1w 10 100 10000 gen_data/
# For the parallel edition the default Input DIR is /mnt/raid/BigDataGeneratorSuite/.
# Output in gen_data DIR
# 
# By: Mingzijian, mingzijian@ict.ac.cn
##

date

# how much parameters at least
PARA_LIMIT=4
if [ "$#" -lt $PARA_LIMIT ]; then
	echo "Usage: $0" "MODEL_NAME" "FIlE_NUM" "FILE_LINES" "LINE_WORDS" "OUT_DATA_DIR"
	exit 1
fi

##
# main part of program
##
MODEL_NAME=$1
i=1
#DATE_MARK=`date '+%Y-%m-%d_%H:%M:%S'`
DATA_DIR=$5
#"gen_data/$DATE_MARK"
PARALLEL_NUM=21
NUMBER_OF_FILES=$2
LINES_PER_FILE=$3
WORDS_PER_LINE=$4
mkdir -p "$DATA_DIR"
c=1
while [ $i -le $NUMBER_OF_FILES ]; do
	/mnt/raid/BigDataGeneratorSuite/Text_datagen/pgen_random_text $MODEL_NAME $LINES_PER_FILE $WORDS_PER_LINE $i > "$DATA_DIR/${MODEL_NAME}_${i}"&
	let i++

	let c++
	if [ $c -eq $PARALLEL_NUM ]; then
		c=1
		wait
	fi
done
wait

date
