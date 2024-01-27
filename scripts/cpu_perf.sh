#!/usr/bin/env bash

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo "      ./cpu.sh [-p][-s][-h]"
    echo
    echo "Options:"
    echo "      -p Performance"
    echo "      -s Power save"
    echo "      -c Conservative"
    echo "      -v View"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Set cpu to performance mode
set_performance() {
  echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
}

# Set cpu to powersave mode
set_powersave() {
  echo powersave | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
}

# Set cpu to conservative mode
set_conservative() {
  echo conservative | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
}

# Check for the input arguments
while getopts ":pscvh" opt
do
  case "${opt}" in
    p)
      set_performance
      exit
      ;;
    s)
      set_powersave
      exit
      ;;
    c)
      set_conservative
      exit
      ;;
    v)
      cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
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

