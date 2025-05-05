
###################################################
#
# file: Makefile_common.mk
#
# @Author:   Iacovos G. Kolokasis
# @Version:  08-04-2023
# @email:    kolokasis@ics.forth.gr
#
###################################################

## Library path
PREFIX := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

## Install path
INSTALL_PREFIX := /usr/local

## Library directories
SRCDIR = $(PREFIX)/src
TESTDIR = $(PREFIX)/tests
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include



## Depended files
LIBHEADERS =  $(INCLUDEDIR)/tera_allocator.h  $(INCLUDEDIR)/tera_assert.h
LIBOBJS = $(SRCDIR)/tera_allocator.o 
TERALIB = $(LIBDIR)/libteraalloc.so

TERA_CREATE_OBJ = $(TESTDIR)/tera_alloc_create.o
TERA_MALLOC_OBJ = $(TESTDIR)/tera_alloc_malloc.o

TERA_CREATE_EXE = tera_alloc_create.bin
TERA_MALLOC_EXE = tera_alloc_malloc.bin

# Detect the platform
PLATFORM := $(shell uname -p)

# Default CC
CC := gcc

# Conditional setting of CC based on platform
#ifeq ($(PLATFORM),x86_64)
#    CC := aarch64-linux-gnu-gcc
#    $(info Detected x86_64 platform, using cross compiler: $(CC))
#else ifeq ($(PLATFORM),aarch64)
#    CC := gcc
#    $(info Detected aarch64 platform, using default compiler: $(CC))
#else
#    $(info Unknown platform: $(PLATFORM), using default compiler: $(CC))
#endif

## Flags
BINFLAG = -c
DEBUGFLAG = -g
OFLAG = -o
WALLFLAG = -Wall -Werror -pedantic
OPTIMZEFLAG = -O3
LDFLAGS = 
CFLAGS = $(BINFLAG) $(WALLFLAG) $(OPTIMIZEFLAG) 


## Commands
RM = rm -fr
AR = ar -r
CP = cp
MKDIR = mkdir -p
