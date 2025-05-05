
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

JAVA_REPO := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))../../../jdk17
# JAVA_COMP=$(JAVA_REPO)/build/base-linux/jdk/bin/javac -Xlint:deprecation -sourcepath ../Whitebox/wb.jar -classpath ./bin/
# JAVA_COMP=$(JAVA_REPO)/build/linux-x86_64-server-slowdebug/jdk/bin/javac -Xlint:deprecation -sourcepath ../Whitebox/wb.jar -classpath bin/
JAVA_COMP=$(JAVA_REPO)/build/linux-x86_64-server-release/jdk/bin/javac -Xlint:deprecation -classpath bin/


JAVAC=$(JAVA_COMP) -d ./bin

GDB=gdb

TARGET=A.class Phase1_MarkOneObject.class Phase1_MarkSubObject.class Phase2_GiveAddressesFromH2.class Phase3_UpdateReferences.class SimpleOneObj.class SimpleOneBackward.class

all: $(TARGET)

%.class: %.java
	$(JAVAC) $<

clean:
	rm -rf out
	rm -f hotspot_pid*

distclean: clean
	rm -rf bin
