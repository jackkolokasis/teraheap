
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

CC = gcc

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
