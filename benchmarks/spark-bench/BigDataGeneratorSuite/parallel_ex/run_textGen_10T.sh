#!/bin/bash
oridir=`pwd`
#cd /mnt/raid/BigDataGeneratorSuite/Text_datagen/
date
echo "Parallel data is being generated..."
pssh -i -h $oridir/conf_hosts -l root -t 0 sh /mnt/raid/BigDataGeneratorSuite/Text_datagen/pgen_text_data.sh  wiki_noSW_90_Sampling 20 833000 10000 /mnt/raid/BigDataGeneratorSuite/gen_text_data/
echo "Finish!"
date
cd $oridir
