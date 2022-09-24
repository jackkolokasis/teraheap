#!/usr/bin/env bash

###################################################
#
# file: grubConfig.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-06-2019
# @email:    kolokasis@ics.forth.gr
#
# Set/Unset grub configuration
#
###################################################

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo "      $0 -s <mem_value> [-u][-h]"
    echo
    echo "Options:"
    echo "      -s  Set memory value (e.g 16g)"
    echo "      -u  Restore to default configuration"
    echo "      -h  Show usage"
    echo

    exit 1
}

##
# Description: 
#   Set memory initial value
#
# Arguments:
#   $1 - Initial memory value 
#
##
set_memory() {
    sudo sed -i 's/quiet/quiet mem='$1'/g' /etc/default/grub
    sudo grub2-mkconfig -o /boot/grub2/grub.cfg
}

##
# Description: 
#   Restore grub configuration
#
##
unset_memory() {
    sudo sed -i '/quiet mem=/c\GRUB_CMDLINE_LINUX="crashkernel=auto rhgb quiet"' /etc/default/grub
    sudo grub2-mkconfig -o /boot/grub2/grub.cfg
}

# Check for the input arguments
while getopts "s:uh" opt
do
    case "${opt}" in
        s)
            echo "Set initial memory value"
            set_memory ${OPTARG}
            exit 1
            ;;
        u)
            echo "Unset grub initial value"
            unset_memory
            exit 1
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done
