#!/usr/bin/env bash

#OLD=1024
#MAX=2048

OLD=1
MAX=2

TOTAL=$(echo $NEW + $OLD | bc)
JAVA="/home/nx05/nx05/kolokasis/teracache/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/java"
JDB="/home/nx05/nx05/kolokasis/teracache/openjdk-8/openjdk8/build/linux-x86_64-normal-server-release/jdk/bin/jdb"
#EXEC=("Array_List" "Simple_Array" "List_Small" "List_Large" "MultiList" \
#	"Simple_Lambda" "Extend_Lambda" "Test_Reflection" "Test_String" "HashMap" \
# 	"Clone" "Rehashing")

EXEC=( "Groupping" )

V_JAVA="/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.292.b10-1.el7_9.x86_64/bin/java"

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
	# Interpreter Tests
	# ${JAVA} \
	#	-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly -XX:+PrintInterpreter -XX:+PrintNMethods \
	#	-Djava.compiler=NONE \
	#	-XX:+ShowMessageBoxOnError \
	#	-XX:+UseParallelGC \
	#	-XX:ParallelGCThreads=1 \
	#	-XX:-UseParallelOldGC \
	#	-XX:-UseCompiler \
	#	-XX:+EnableTeraCache \
	#	-XX:TeraCacheSize=524288000  \
	#	-Xmx${MAX}g \
	#	-Xms${OLD}m \
	#	-Xmn200m \
	#	-XX:-UseCompressedOops \
	#	-XX:+TeraCacheStatistics \
	#	-Xlogtc:llarge_teraCache.txt \
	#	${exec_file}  > err 2>&1 > out
	 
	# C1 Tests
	# ${JAVA} \
	#	-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly \
	#	-XX:+PrintInterpreter \
	#	-XX:+PrintNMethods -XX:+PrintCompilation \
	#	-XX:+ShowMessageBoxOnError -XX:+LogCompilation \
	#	-XX:TieredStopAtLevel=3\
	#	-XX:+UseParallelGC \
	#	-XX:ParallelGCThreads=1 \
	#	-XX:-UseParallelOldGC \
	#	-XX:+EnableTeraCache \
	#	-XX:TeraCacheSize=524288000 \
	#	-Xmx${MAX}g \
	#	-Xms${OLD}m \
	#	-Xmn200m \
	#	-XX:-UseCompressedOops \
	#	-XX:+TeraCacheStatistics \
	#	-Xlogtc:llarge_teraCache.txt \
	#	${exec_file} > err 2>&1 > out
	 
	 # C2 Tests
	# ${JAVA} \
	#	 -server \
	#	-XX:+UnlockDiagnosticVMOptions -XX:+PrintAssembly \
	#	-XX:+PrintNMethods -XX:+PrintCompilation \
	#	-XX:+ShowMessageBoxOnError -XX:+LogCompilation \
	#	-XX:+UseParallelGC \
	#	-XX:ParallelGCThreads=16 \
	#	-XX:-UseParallelOldGC \
	#	-XX:+EnableTeraCache \
	#	-XX:TeraCacheSize=1073741824 \
	#	-Xmx${MAX}g \
	#	-Xms${OLD}g \
	#	-XX:TeraCacheThreshold=123030 \
	#	-XX:-UseCompressedOops \
	#	-XX:+TeraCacheStatistics \
	#	-Xlogtc:llarge_teraCache.txt \
	#	${exec_file} > err 2>&1 > out
	 
	 # C2 Tests
	 ${JAVA} \
		 -server \
		-XX:+ShowMessageBoxOnError \
		-XX:+UseParallelGC \
		-XX:ParallelGCThreads=1 \
		-XX:-UseParallelOldGC \
		-XX:+EnableTeraCache \
		-XX:TeraCacheSize=1073741824 \
		-Xmx${MAX}g \
		-Xms${OLD}g \
		-XX:TeraCacheThreshold=0 \
		-XX:-UseCompressedOops \
		-XX:+TeraCacheStatistics \
		-Xlogtc:llarge_teraCache.txt \
		${exec_file} > err 2>&1 > out
			
	# Vanilla
	# ${V_JAVA} \
	# 	 -server \
	# 	-XX:+UseParallelGC \
	# 	-XX:ParallelGCThreads=16 \
	# 	-XX:-UseParallelOldGC \
	# 	-Xmx${OLD}m \
	# 	-Xms${OLD}m \
	# 	-Xmn200m \
	# 	-XX:-UseCompressedOops \
	# 	${exec_file} > err 2>&1 > out

	 # C2 Tests
	# ${JAVA} \
	#	 -server \
	#	-XX:+UseParallelGC \
	#	-XX:ParallelGCThreads=16 \
	#	-XX:-UseParallelOldGC \
	#	-XX:+EnableTeraCache \
	#	-XX:TeraCacheSize=524288000 \
	#	-Xmx${MAX}m \
	#	-Xms${OLD}m \
	#	-Xmn200m \
	#	-XX:-UseCompressedOops \
	#	-XX:+TeraCacheStatistics \
	#	-Xlogtc:llarge_teraCache.txt \
	#	${exec_file} > err 2>&1 > out

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
