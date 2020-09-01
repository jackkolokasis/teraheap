#!/usr/bin/env bash

NEW=5
OLD=5
CACHE=0
TOTAL=$(echo $NEW + $OLD + $CACHE | bc)

JAVAC="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/javac"
JAVA="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/java"
JAVAP="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/javap"

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -c  Compile"
    echo "      -r  Run"
    echo "      -p  Analysis"
    echo "      -g  Debug"
    echo "      -d  Delete produced files"
    echo "      -h  Show usage"
    echo

    exit 1
}

function run()
{
	#    ~/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/java \
		#        -Xint \
		#        -XX:+UseParallelGC \
		#        -XX:+CacheGenerationEnabled \
		#        -XX:NewSize=$NEW \
		#        -XX:MaxNewSize=$NEW \
		#        -XX:OldSize=$OLD \
		#        -XX:CacheSize=$CACHE \
		#        -XX:MaxCacheSize=$CACHE \
		#        -XX:-UseAdaptiveSizePolicy \
		#        -Xms${TOTAL}g \
		#        -Xmx${TOTAL}g \
		#        -XX:AllocateOldGenAt=/dev/sdd \
		#        TestMigration

	${JAVA} \
		-XX:+UseParallelGC \
		-XX:-UseParallelOldGC \
		-Xmn${NEW}g \
		-Xms${TOTAL}g \
		-Xmx${TOTAL}g \
		-XX:-UseCompressedOops \
		-XX:+EnableTeraCache $1 > err 2>&1 > out
}

function compile()
{
	${JAVAC} $1
}

function analysis()
{
	${JAVAP} -c -p -v $1
}

function debug()
{
	gdb ${JAVA} $1
}

function delete()
{
	rm core.*
	rm hs_err_pid*
	rm err out
}

while getopts ":crpdgh" opt
do
	case "${opt}" in
		c)
			compile $2
			;;
		r)
			run $2
			;;
		p)
			analysis $2
			;;
		g)
			debug $2
			;;
		d)
			delete
			;;
		h)
			usage
			;;
		*)
			usage
			;;
	esac
done
