
###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  03-09-2018
# @email:    kolokasis@ics.forth.gr
#
# Makefile create libmemalloc.a static library
#
#####################################################

CC = gcc
CFLAGS= -fPIC -O3 -g -Wall -c
OBJ_DIR = ./obj
OUT_DIR = ./lib
TARGET = libpmemalloc.so

all: $(TARGET)

# Enumarating every .c as .o and using it as depedency
$(TARGET): $(patsubst %.c,./obj/%.o,$(wildcard *.c)) 
	$(CC) -shared $^ -o $(OUT_DIR)/$@ 
	#$(AR) rv -o $(OUT_DIR)/$@ $^ 

# Compiling every *.c to *.o
$(OBJ_DIR)/%.o: %.c dirmake
	$(CC) -c $(CFLAGS) $< -o $@

dirmake:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OUT_DIR)

clean:
	rm -f $(OBJ_DIR)/*.o 

distclean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)

rebuild: clean all

help:
	@echo Makefile Use:
	@echo -------------
	@echo
	@echo ' ' "make all:" '\t''\t'build the library
	@echo ' ' "make clean:" '\t''\t'delete all intermediate files \(*.o, etc\)
	@echo ' ' "make distclean:" '\t'delete the executables
	@echo ' ' "make rebuild:" '\t'clean all intermediate files and build the library
