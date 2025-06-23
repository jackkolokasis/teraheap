#!/bin/bash

set -e

echo " Load eBPF objects..."

sudo ./load_bpf_setup
sudo chown -R kolokasis /sys/fs/bpf
