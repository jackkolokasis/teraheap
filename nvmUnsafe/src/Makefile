
###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-08-2018
# @email:    kolokasis@ics.forth.gr
#
# Makefile
#
###################################################

JAVAC=javac
JAVAH=javah
JAVA=java
CC=gcc
JAVA_PATH=/usr/lib/jvm/java-8-oracle
OUT_DIR=lib
OBJ_DIR=obj
FLAGS=-Djava.library.path=$(OUT_DIR)
LD_LIBRARY_PATH=../libpmemmalloc/lib

all: $(OUT_DIR)/libNVMUnsafe.so

# $@ matches the target, $< matches the first dependency
$(OUT_DIR)/libNVMUnsafe.so: $(OBJ_DIR)/NVMUnsafe.o 
	$(CC) -shared $< -L../libpmemmalloc/lib/ -lpmemalloc -lpmem -o $@

$(OBJ_DIR)/NVMUnsafe.o: NVMUnsafe.c NVMUnsafe.h dirmake
	$(CC) -fpic -I"$(JAVA_PATH)/include/" -I"$(JAVA_PATH)/include/linux/" -c $< -o $@ 

# $* matches the target filename without the extension
NVMUnsafe.h: NVMUnsafe.class
	$(JAVAH) $*

NVMUnsafe.class: NVMUnsafe.java
	$(JAVAC) NVMUnsafe.java

run:
	@export LD_LIBRARY_PATH=$(LD_LIBRARY_PATH)
	$(JAVA) $(FLAGS) NVMUnsafe

dirmake:
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJ_DIR)

clean :
	rm -rf NVMUnsafe.h $(OBJ_DIR) $(OUT_DIR) NVMUnsafe.class

rebuild: clean all

help:
	@echo Makefile Use:
	@echo -------------
	@echo 
	@echo \ \ "make all" '\t' build native shared library \(libpmemalloc.so\)
	@echo \ \ "make run" '\t' run the test
	@echo \ \ "make clean" '\t' to delete all intermediate files \(*.o, etc\)
	@echo \ \ "make rebuild" '\t' to clean all intermediate files and build the library
	@echo \ \ "make help" '\t' help
