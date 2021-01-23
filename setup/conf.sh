#!/usr/bin/env bash

#Unload  module nvme
sudo modprobe -r nvme nvme_core

# Restart mellanox module 
sudo service openibd restart

# Load (again) nvme
sudo modprobe nvme

sudo swapoff -a
sudo /tmp/ramdisk_create_and_mount.sh -c
dd if=/dev/zero of=/mnt/ramdisk/dummy.txt bs=1M count=205824

