#!/bin/bash
oridir=`pwd`
#cd /mnt/raid/BigDataGeneratorSuite/Table_datagen/
date
echo "Parallel data is being generated..."
pssh -i -h $oridir/conf_hosts -l root -t 0 sh /mnt/raid/BigDataGeneratorSuite/Table_datagen/personal_generator/pgen_resume.sh 85125000 20 /mnt/raid/BigDataGeneratorSuite/gen_personal_data/
echo "Finish!"
date
cd $oridir
