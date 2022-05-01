#!/usr/bin/env bash

###################################################
#
# file: dev_setup.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  26-02-2022
# @email:    kolokasis@ics.forth.gr
#
# Prepare the devices for the experiments
#
###################################################

. ./conf.sh

# Check if the last command executed succesfully
#
# if executed succesfully, print SUCCEED
# if executed with failures, print FAIL and exit
check () {
    if [ $1 -ne 0 ]
    then
        echo -e "  $2 \e[40G [\e[31;1mFAIL\e[0m]" >> $LOG
        exit
    else
        echo -e "  $2 \e[40G [\e[32;1mSUCCED\e[0m]" >> $LOG
    fi
}

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
	echo "      -t  Run experiments with TeraHeap"
    echo "      -d  Unmount all devices"
    echo "      -h  Show usage"
    echo

    exit 1
}

##
# Description: 
#   Delete TeraHeap file all mount points
#
##

destroy_th() {
		rm -rf ${TH_DIR}/file.txt
		retValue=$?
		message="Remove TeraCache file" 
		check ${retValue} "${message}"
		
		rm -rf ${ZOOKEEPER_DIR}/*
		retValue=$?
		message="Remove Zookeeper files" 
		check ${retValue} "${message}"
		
		sudo umount ${TH_DIR}
		retValue=$?
		message="Unmount $DEV_TH" 
		check ${retValue} "${message}"
		
		sudo umount ${ZOOKEEPER_DIR}
		retValue=$?
		message="Unmount $DEV_ZK" 
		check ${retValue} "${message}"
}

destroy_ser() {
	rm -rf ${ZOOKEEPER_DIR}/*
	retValue=$?
	message="Remove Zookeeper files" 
	check ${retValue} "${message}"

	sudo umount ${ZOOKEEPER_DIR}
	retValue=$?
	message="Unmount $DEVICE_ZK" 
	check ${retValue} "${message}"
}
    
# Check for the input arguments
while getopts "tdh" opt
do
    case "${opt}" in
		t)
			TC=true
			;;
		d)
			DESTROY=true
			;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

# Unmount TeraCache device
if [ $DESTROY ]
then
	if [ $TC ]
	then
		destroy_tc
	else
		destroy_ser
	fi
	exit
fi

# Setup TeraCache device
if [ $TC ]
then
	if ! mountpoint -q ${ZOOKEEPER_DIR}
	then
		# Setup disk for zookeeper
		sudo mount /dev/${DEV_ZK} ${ZOOKEEPER_DIR}
		# Check if the command executed succesfully
		retValue=$?
		message="Mount ${DEV_ZK} for zookeeper" 
		check ${retValue} "${message}"

		sudo chown kolokasis ${ZOOKEEPER_DIR}
		# Check if the command executed succesfully
		retValue=$?
		message="Change ownerships /mnt/spark" 
		check ${retValue} "${message}"
	fi

	rm -rf ${ZOOKEEPER_DIR}/*

	if ! mountpoint -q ${TH_DIR}
	then
		# Setup disk for zookeeper
		sudo mount /dev/${DEV_TH} ${TH_DIR}
		# Check if the command executed succesfully
		retValue=$?
		message="Mount ${DEV_TH} for TeraHeap" 
		check ${retValue} "${message}"

		sudo chown kolokasis ${TH_DIR}
		# Check if the command executed succesfully
		retValue=$?
		message="Change ownerships /mnt/fmap" 
		check ${retValue} "${message}"
	fi

	cd ${TH_DIR}

	# if the file does not exist then create it
	if [ ! -f file.txt ]
	then
		fallocate -l ${TH_FILE_SZ}G file.txt
		retValue=$?
		message="Create ${TH_FILE_SZ}G file for TeraHeap" 
		check ${retValue} "${message}"
	else
		rm -rf file.txt
		
		fallocate -l ${TH_FILE_SZ}G file.txt
		retValue=$?
		message="Create ${TC_FILE_SZ}G file for TeraHeap" 
		check ${retValue} "${message}"
	fi
	cd - >> ${LOG} 2>&1
else
	if ! mountpoint -q ${ZOOKEEPER_DIR}
	then
		# Setup disk for zookeeper
		sudo mount /dev/${DEV_ZK} ${ZOOKEEPER_DIR}
		# Check if the command executed succesfully
		retValue=$?
		message="Mount ${DEV_ZK} for zookeeper" 
		check ${retValue} "${message}"

		sudo chown kolokasis ${ZOOKEEPER_DIR}
		# Check if the command executed succesfully
		retValue=$?
		message="Change ownerships /mnt/spark" 
		check ${retValue} "${message}"
	fi

	rm -rf ${ZOOKEEPER_DIR}/*
fi
exit
