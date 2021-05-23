#!/usr/bin/env bash

###################################################
#
# file: prepare.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  07-03-2021 
# @email:    kolokasis@ics.forth.gr
#
# Prepare spark and jvm for experiments
#
###################################################


# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -s  Run experiments with serialization"
    echo "      -t  Run experiments with TeraCache"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts "tsh" opt
do
    case "${opt}" in
		s)
			SERDES=true
			;;
		t)
			TERACACHE=true
			;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

if [ $SERDES ]
then
	cd /usr/lib/jvm

	check=$(ls | grep -w "java-8-kolokasis_stable")

	if [ $check ]
	then
		sudo mv java-8-kolokasis java-8-kolokasis_tc
		sudo mv java-8-kolokasis_stable java-8-kolokasis
	fi

	cd -

	cd /opt/spark/spark-2.3.0-kolokasis/conf

	check=$(ls | grep "spark-defaults.conf.vanilla.bk")

	if [ "$check" ]
	then
		mv spark-defaults.conf spark-defaults.conf.tc.bk
		mv spark-defaults.conf.vanilla.bk spark-defaults.conf
	fi

	cd -
elif [ $TERACACHE ]
then
	cd /usr/lib/jvm

	check=$(ls | grep "java-8-kolokasis_tc")

	if [ $check ]
	then
		sudo mv java-8-kolokasis java-8-kolokasis_stable
		sudo mv java-8-kolokasis_tc java-8-kolokasis
	fi

	cd -

	cd /opt/spark/spark-2.3.0-kolokasis/conf

	check=$(ls | grep "spark-defaults.conf.tc.bk")

	if [ "$check" ]
	then
		mv spark-defaults.conf spark-defaults.conf.vanilla.bk
		mv spark-defaults.conf.tc.bk spark-defaults.conf
	fi

	cd -
fi
