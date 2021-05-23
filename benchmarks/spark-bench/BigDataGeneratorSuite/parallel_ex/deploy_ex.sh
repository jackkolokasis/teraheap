#!/bin/bash
echo "The deployment process is being start..."
pssh -i -h conf_hosts -l root -t 0 rm -rf /mnt/raid/BigDataGeneratorSuite/ 
echo "Cleanup target directory!"
pssh -i -h conf_hosts -l root -t 0 mkdir -p /mnt/raid/BigDataGeneratorSuite/
echo "Make target directory!"
pscp -h conf_hosts -r -l root -t 0 /mnt/raid/BigDataGeneratorSuite/ /mnt/raid/
echo "Finish!"
