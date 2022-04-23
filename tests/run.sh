#!/usr/bin/env bash

XMS=2
MAX=100
TERACACHE_SIZE=$(echo $(( (${MAX}-${XMS})*1024*1024*1024 )))
PARALLEL_GC_THREADS=16
PLATFORM=""
STRIPE_SIZE=32768

if [ $PLATFORM == "nextgenio" ] 
then
    JAVA="/home/nx05/nx05/kolokasis/teracache/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/java"
    JDB="/home/nx05/nx05/kolokasis/teracache/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/jdb"

    EXEC=( "Groupping" )
else 
    JAVA="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/java"
    JDB="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/jdb"

	EXEC=("Array" "Array_List" "Array_List_Int" "List_Large" "MultiList" \
		"Simple_Lambda" "Extend_Lambda" "Test_Reflection" "Test_Reference" \
		"HashMap" "Rehashing" "Clone" "Groupping" "MultiHashMap" \
		"Test_WeakHashMap" "ClassInstance")

	EXEC=( "HashMap" )
fi
V_JAVA="/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.292.b10-1.el7_9.x86_64/bin/java"

# Run tests using only interpreter mode
function interpreter_mode() {
	${JAVA} \
		-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly -XX:+PrintInterpreter -XX:+PrintNMethods \
		-Djava.compiler=NONE \
		-XX:+ShowMessageBoxOnError \
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=${PARALLEL_GC_THREADS} \
		-XX:-UseParallelOldGC \
		-XX:-UseCompiler \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=${TERACACHE_SIZE} \
		-Xmx${MAX}g \
		-Xms${XMS}m \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-Xlogtc:llarge_teraCache.txt $1 > err 2>&1 > out
}

# Run tests using only C1 compiler
function c1_mode() {
	 ${JAVA} \
		-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly \
		-XX:+PrintInterpreter \
		-XX:+PrintNMethods -XX:+PrintCompilation \
		-XX:+ShowMessageBoxOnError -XX:+LogCompilation \
		-XX:TieredStopAtLevel=3\
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=${PARALLEL_GC_THREADS} \
		-XX:-UseParallelOldGC \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=${TERACACHE_SIZE} \
		-Xmx${MAX}g \
		-Xms${XMS}m \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-Xlogtc:llarge_teraCache.txt $1 > err 2>&1 > out
}
	 
# Run tests using C2 compiler
function c2_mode() {
	 ${JAVA} \
		 -server \
		-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly \
		-XX:+PrintNMethods -XX:+PrintCompilation \
		-XX:+ShowMessageBoxOnError -XX:+LogCompilation \
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=${PARALLEL_GC_THREADS} \
		-XX:-UseParallelOldGC \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=${TERACACHE_SIZE} \
		-Xmx${MAX}g \
		-Xms${XMS}g \
		-XX:TeraCacheThreshold=0 \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-Xlogtc:llarge_teraCache.txt $1 > err 2>&1 > out
} 

# Run tests using all compilers
function run_tests() {
	${JAVA} \
		-server \
		-XX:+ShowMessageBoxOnError \
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=${PARALLEL_GC_THREADS} \
		-XX:-UseParallelOldGC \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=${TERACACHE_SIZE} \
		-Xmx${MAX}g \
		-Xms${XMS}g \
		-XX:TeraCacheThreshold=0 \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-XX:TeraStripeSize=${STRIPE_SIZE} \
		-Xlogtc:llarge_teraCache.txt $1 > err 2>&1 > out
}

# Run tests using gdb
function run_tests_debug() {
	gdb --args ${JAVA} \
		-server \
		-XX:+ShowMessageBoxOnError \
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=${PARALLEL_GC_THREADS} \
		-XX:-UseParallelOldGC \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=${TERACACHE_SIZE} \
		-Xmx${MAX}g \
		-Xms${XMS}g \
		-XX:TeraCacheThreshold=0 \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-XX:TeraStripeSize=${STRIPE_SIZE} \
		-Xlogtc:llarge_teraCache.txt $1
}

cd java
make clean;

clear
echo "___________________________________"
echo 
echo "         Run JAVA Tests"
echo "___________________________________"
echo 

for exec_file in "${EXEC[@]}"
do
	case $1 in
		1)
			interpreter_mode $exec_file
			;;
		2)
			c1_mode $exec_file
			;;
		3)
			c2_mode $exec_file
			;;
		4)
			run_tests_debug $exec_file
			;;
		*)
			run_tests $exec_file
			;;
	esac

	ans=$?

	echo -ne "${exec_file} "

	if [ $ans -eq 0 ]
	then    
		echo -e '\e[30G \e[32;1mPASS\e[0m';    
	else    
		echo -e '\e[30G \e[31;1mFAIL\e[0m';    
		break
	fi    
done

cd -
