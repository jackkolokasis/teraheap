###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-10-2018
# @email:    kolokasis@ics.forth.gr
#
# Makefile build the example
#
####################################################

JAVAC=javac
JAVA=java
OUT_DIR=../src/lib
FLAGS=-Djava.library.path=$(OUT_DIR)
_LD_LIBRARY_PATH=../libpmemmalloc/lib

all: Example.class
	export LD_LIBRARY_PATH=$(_LD_LIBRARY_PATH);\
	echo $$LD_LIBRARY_PATH
	$(JAVA) $(FLAGS) $*

Example.class: Example.java
	$(JAVAC) $<


