#!/usr/bin/env bash

# COMPLETION_NSEC=1000000000 # 1sec
# COMPLETION_NSEC=500000000  # 500ms
COMPLETION_NSEC=10000000     # 10ms

sudo modprobe null_blk \
  nr_devices=1 \
  irqmode=2 \
  completion_nsec=${COMPLETION_NSEC} \
  memory_backed=1

sudo chown -R kolokasis /dev/nullb0
