
###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  08-09-2018
# @email:    kolokasis@ics.forth.gr
#
# pmemalloc test files makefile
#
###################################################

CC = gcc
FLAGS = -lpmemalloc -lpmem
LIBRARY_PATH = ../lib
TARGET = test_exe

all: $(TARGET) 

test_exe : test.c
	$(CC) $< -o $@ -L$(LIBRARY_PATH) $(FLAGS)

clean:
	rm -rf /mnt/mem/file

distclean:
	rm -rf $(TARGET) 

help:
	@echo Makefile Usage:
	@echo --------------
	@echo 
	@echo \'make all\' 	 Build all tests
	@echo \'make clean\' Remove intermediate files
	@echo \'make disclean\' Remove executable files
	@echo 
