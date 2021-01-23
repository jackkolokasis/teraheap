#!/usr/bin/env bash

#Define devices
SPARK_DEV="/dev/nvme0n1"
SPARK_DIR="/mnt/nvme"


#Define some variables for pretty printing
ESC="\033["
RESET="${ESC}m"
BOLD=1
RED_FG=31
GREEN_FG=32
BRED="${ESC}${BOLD};${RED_FG}m"
BGREEN="${ESC}${BOLD};${GREEN_FG}m"

# Mount nvme
if mountpoint -q ${SPARK_DIR}
then
    echo -e "[${BRED}FAIL${RESET}]\t\t Already mounted nvme0n1"
else
    echo -e "[${BGREEN}SUCCESS${RESET}]\t Mount nvme0n1"
    sudo mount ${SPARK_DEV} ${SPARK_DIR}
    sudo chmod 777 ${SPARK_DIR}
fi
