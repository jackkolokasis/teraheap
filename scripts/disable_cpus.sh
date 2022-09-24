#!/usr/bin/env bash

START=4
END=31

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo "      ./cpu.sh [-e][-d][-h]"
    echo
    echo "Options:"
    echo "      -e  Enable cpus"
    echo "      -d  Disable cpus"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Enable Cores
enable_cores() {
    for i in `seq ${START} ${END}`;
    do
        sudo echo 1 > /sys/devices/system/cpu/cpu$i/online
    done
}

# Disable Cores
disable_cores() {
    for i in `seq ${START} ${END}`;
    do
        sudo echo 0 > /sys/devices/system/cpu/cpu$i/online
    done
}

# Check for the input arguments
while getopts ":edh" opt
do
    case "${opt}" in
        e)
            enable_cores
            exit
            ;;
        d)
            disable_cores
            exit
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

