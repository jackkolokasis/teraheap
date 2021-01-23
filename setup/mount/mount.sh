#!/usr/bin/env bash

# Mount spark directory
if mountpoint -q /mnt/spark
then
    echo "SPARK: Already Mounted"
else
    echo "/mnt/spark mount succesfully"
    sudo mount /dev/sdg /mnt/spark
fi
    
# Print new line
echo

# Mount nvme
if mountpoint -q /mnt/nvme
then
    echo "NVME: Already Mounted"
else
    echo "/mnt/nvme mount succesfully"
    sudo mount /dev/sdf /mnt/nvme
    sudo chmod 777 /mnt/nvme
fi
