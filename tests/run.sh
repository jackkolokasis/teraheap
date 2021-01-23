#!/usr/bin/env bash

OLD=1
MAX=10
TOTAL=$(echo $NEW + $OLD | bc)
JAVA="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/java"
JDB="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/jdb"
#EXEC=("Array_List" "Simple_Array" "List_Small" "List_Large" "MultiList" "Simple_Lambda" "Extend_Lambda" "Test_Reflection" "Test_String")
EXEC=("HashMap")

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
	 ${JAVA} \
		-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly -XX:+PrintInterpreter -XX:+PrintNMethods \
		-Djava.compiler=NONE \
		-XX:+ShowMessageBoxOnError \
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=1 \
		-XX:-UseParallelOldGC \
		-XX:-UseCompiler \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=9663676416 \
		-Xmx${MAX}g \
		-Xms${OLD}g \
		-Xmn200m \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-Xlogtc:llarge_teraCache.txt \
		${exec_file} > err 2>&1 > out

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
