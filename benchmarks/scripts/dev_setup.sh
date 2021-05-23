#!/usr/bin/env bash

###################################################
#
# file: dev_setup.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  28-03-2021 
# @email:    kolokasis@ics.forth.gr
#
# Prepare the devices for the experiments
#
###################################################

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -f  Run experiments with fastmap"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts "fh" opt
do
    case "${opt}" in
		f)
			FASTMAP=true
			;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

# Setup disk for shuffle
sudo mount /dev/nvme1n1 /mnt/nvme
sudo chown kolokasis /mnt/nvme

# Setup TeraCache device
if [ $FASTMAP ]
then
	sudo chown kolokasis /mnt/fmap
	cd /mnt/fmap

	# if the file does not exist then create it
	if [ ! -f file.txt ]
	then
		fallocate -l 200G file.txt
	fi
	cd -
else
	sudo mount /dev/nvme0n1 /mnt/spark
	sudo chown kolokasis /mnt/spark

	cd /mnt/spark

	# if the file does not exist then create it
	if [ ! -f file.txt ]
	then
		dd if=/dev/zero of=file.txt bs=1M count=204800
	fi
	cd -
fi

exit
