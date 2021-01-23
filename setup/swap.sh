#!/usr/bin/env bash

if mountpoint -q /mnt/spark
then
    echo "It;s mounted"
else
    echo "It's not mounted"
    sudo mount /dev/nvme1n1 /mnt/spark

    if [ $? -eq 0 ]
    then
        echo "Mount success"
    else
        echo "Mount unsuccess"
        sudo mkfs.xfs /dev/nvme1n1
        if [ $? -eq 0 ]
        then
            sudo mount /dev/nvme1n1 /mnt/spark
            if [ $? -eq 0 ]
            then
                echo "Mount success"
            else
                echo "Exit..."
                exit
            fi
        else
            echo "Exit..."
            exit
        fi
    fi
fi


## Create a file to be used as swap space
sudo dd if=/dev/zero of=/mnt/spark/swapfile bs=1M count=409600

## Ensure root user can read and write to the swap file
sudo chmod 600 /mnt/spark/swapfile

## Set up linux swap area on the file
sudo mkswap /mnt/spark/swapfile

## Activate swap
sudo swapon /mnt/spark/swapfile
