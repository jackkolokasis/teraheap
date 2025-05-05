
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
JAVAC=$(JAVA_REPO)/build/linux-$(cpu_arch)-server-release/jdk/bin/javac
PROJECT_DIR := $(shell pwd)/../..

GDB=gdb

TARGET=A.class Phase1_MarkOneObject.class Phase1_MarkSubObject.class Phase2_GiveAddressesFromH2.class Phase3_UpdateReferences.class SimpleOneObj.class SimpleOneBackward.class

all: $(TARGET)

%.class: %.java
	$(JAVAC) $<

clean:
	rm core.*
	rm hs_err_pid*
	rm -rf err out
	rm -f hotspot_pid*

distclean: clean
	rm -rf *.class
