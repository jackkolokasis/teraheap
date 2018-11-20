###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-10-2018
# @email:    kolokasis@ics.forth.gr
#
# Makefile build the example for the test
#
####################################################

JAVAC		= javac
JAVA		= java
JAR			= NVMUnsafe.jar
CLASSPATH 	= ".:../NVMUnsafe.jar"

all: Example.class
	$(JAVA) -cp $(CLASSPATH) Example

Example.class: Example.java
	$(JAVAC) -cp $(CLASSPATH) $<

clean:
	rm -rf Example.class

