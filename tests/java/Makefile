
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

JAVA_REPO="/home1/public/kolokasis/sparkPersistentMemory/openjdk-8/openjdk8"
JAVAC=$(JAVA_REPO)/build/linux-x86_64-normal-server-release/jdk/bin/javac
JAVAP=$(JAVA_REPO)/build/linux-x86_64-normal-server-release/jdk/bin/javap
JAVA=$(JAVA_REPO)/build/linux-x86_64-normal-server-release/jdk/bin/java
GDB=gdb

TARGET=Array_List.class Array_List_Int.class Simple_Array.class List_Small.class List_Large.class \
	   MultiList.class Simple_Lambda.class Extend_Lambda.class Test_Reflection.class \
	   Test_String.class Array_List_Scalar.class Test_Reference.class \
	   HashMap.class Array.class Rehashing.class Clone.class
	
all: $(TARGET)

Array_List.class:	Array_List.java 
	$(JAVAC) $<

Array_List_Int.class:	Array_List_Int.java 
	$(JAVAC) $<

Array_List_Float.class:	Array_List_Float.java 
	$(JAVAC) $<

Array_List_Double.class:	Array_List_Double.java 
	$(JAVAC) $<

Array_List_Scalar.class:	Array_List_Scalar.java 
	$(JAVAC) $<

Simple_Array.class: Simple_Array.java 
	$(JAVAC) $<

List_Small.class: List_Small.java 
	$(JAVAC) $<

List_Large.class: List_Large.java 
	$(JAVAC) $<

MultiList.class: MultiList.java 
	$(JAVAC) $<

Simple_Lambda.class: Simple_Lambda.java 
	$(JAVAC) $<

Extend_Lambda.class: Extend_Lambda.java 
	$(JAVAC) $<

Test_Reflection.class: Test_Reflection.java
	$(JAVAC) $<
		
Test_String.class: Test_String.java 
	$(JAVAC) $<

Test_Reference.class: Test_Reference.java 
	$(JAVAC) $<

HashMap.class: HashMap.java 
	$(JAVAC) -g $<

Array.class: Array.java
	$(JAVAC) $<

Rehashing.class: Rehashing.java
	$(JAVAC) $<

Clone.class: Clone.java
	$(JAVAC) $<

debug:
	-$(GDB) $(JAVA) $f

clean:
	-rm core.*
	-rm hs_err_pid*
	-rm err out
	-rm hotspot_pid*
	-rm replay_pid*

distclean: clean
	-rm -f *.class

analysis:
	-${JAVAP} -c -p -v $f
