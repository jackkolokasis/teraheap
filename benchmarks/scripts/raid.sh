#!/usr/bin/env bash

#mdadm --stop /dev/md0
#mdadm --zero-superblock /dev/nvme0n1 /dev/nvme1n1

# blkdiscard /dev/nvme0n1
# blkdiscard /dev/nvme1n1

sudo mdadm --create --verbose /dev/md0 --level=stripe --chunk=1024 --raid-devices=2 /dev/nvme0n1 /dev/nvme1n1

