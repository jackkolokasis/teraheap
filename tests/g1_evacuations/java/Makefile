
###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  20-09-2020
# @email:    kolokasis@ics.forth.gr
#
# Makefile for test files
#
####################################################
cpu_arch=$(shell uname -p)

JAVA_REPO := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))/../../../jdk17u067
#JAVAC=$(JAVA_REPO)/build/linux-$(cpu_arch)-server-release/jdk/bin/javac -Xlint:deprecation -sourcepath ../Whitebox/wb.jar -classpath
#PROJECT_DIR := $(shell pwd)/../..

JAVA_COMP=$(JAVA_REPO)/build/linux-$(cpu_arch)-server-release/jdk/bin/javac -Xlint:deprecation -sourcepath ../Whitebox/wb.jar -classpath ./bin/
JAVAC=$(JAVA_COMP) -d ./bin

GDB=gdb

TARGET=GC.class Array.class Array_List.class Array_List_Int.class \
			 Array_List_String.class Array_mine.class List_Large.class \
			 MultiList.class Simple_Lambda.class Extend_Lambda.class \
			 Test_Reflection.class Test_Reference.class HashMap.class Rehashing.class \
			 Clone.class Groupping.class MultiHashMap.class Test_WeakHashMap.class \
			 ClassInstance.class
	   
all: $(TARGET)

%.class: %.java
	$(JAVAC) $<

clean:
	rm -r out
	rm hotspot_pid*

distclean: clean
	rm -rf bin
