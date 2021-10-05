#!/usr/bin/env bash

DEVICE="nvme1n1"
FILE_SIZE="128"

# Check if the last command executed succesfully
#
# if executed succesfully, print SUCCEED
# if executed with failures, print FAIL and exit
check () {
    if [ $1 -ne 0 ]
    then
        echo -e "  $2 \e[40G [\e[31;1mFAIL\e[0m]"
        exit
    else
        echo -e "  $2 \e[40G [\e[32;1mSUCCED\e[0m]"
    fi
}

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-h]"
    echo
    echo "Options:"
    echo "      -c  Prepare the eviroment for TeraCache"
    echo "      -d  Unload the enviroment"
    echo "      -h  Show usage"
    echo

    exit 1
}

create() {
	clear
	echo "___________________________________"
	echo 
	echo -e "\e[32;1mPrepare Enviroment for TeraCache\e[0m "
	echo "___________________________________"
	echo 

	sudo mkdir -p /mnt/fmap

	# Check if the command executed succesfully
	retValue=$?
	message="Create directory for TeraCache" 
	check ${retValue} "${message}"
	
	sudo chown kolokasis /mnt/fmap
	retValue=$?
	message="Change ownerships" 
	check ${retValue} "${message}"
	
	sudo mount /dev/$DEVICE /mnt/fmap
	retValue=$?
	message="Mount $DEVICE to /mnt/fmap" 
	check ${retValue} "${message}"
	
	fallocate -l ${FILE_SIZE}G /mnt/fmap/file.txt
	retValue=$?
	message="Create ${FILE_SIZE}G file for TeraCache " 
	check ${retValue} "${message}"
}

destroy() {
	clear
	echo "___________________________________"
	echo 
	echo -e "\e[32;1mDestroy Enviroment for TeraCache\e[0m "
	echo "___________________________________"
	echo 

	rm -rf /mnt/fmap/file.txt
	
	# Check if the command executed succesfully
	retValue=$?
	message="Delete file" 
	check ${retValue} "${message}"
	
	sudo umount /mnt/fmap
	retValue=$?
	message="Unmount $DEVICE" 
	check ${retValue} "${message}"
}

# Check for the input arguments
while getopts "cdh" opt
do
    case "${opt}" in
        c)
			create
            ;;
        d)
			destroy
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done
