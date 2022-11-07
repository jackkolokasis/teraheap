
###################################################
#
# file: Makefile_common.mk
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-03-2021 
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
LIBHEADERS =  $(INCLUDEDIR)/regions.h $(INCLUDEDIR)/asyncIO.h $(INCLUDEDIR)/segments.h
LIBREGIONSOBJS = $(SRCDIR)/regions.o $(SRCDIR)/asyncIO.o $(SRCDIR)/segments.o
REGIONSLIBRARY = $(LIBDIR)/libregions.so

TC_ALLOCATE_OBJ = $(TESTDIR)/tc_allocate.o
TC_GROUP_OBJ = $(TESTDIR)/tc_group.o
TC_FREE_OBJ = $(TESTDIR)/tc_free.o
TC_SYNC_OBJ = $(TESTDIR)/tc_sync_write.o
TC_ASYNC_OBJ = $(TESTDIR)/tc_async_write.o
TC_ALLOCATE_MULTI_REGION = $(TESTDIR)/tc_allocate_multi_regions.o

TC_ALLOCATE_EXE = tc_allocate.bin
TC_GROUP_EXE = tc_group.bin
TC_FREE_EXE = tc_free.bin
TC_SYNC_EXE = tc_sync_write.bin
TC_ASYNC_EXE = tc_async_write.bin
TC_ALLOCATE_MULTI_REGION_EXE = tc_allocate_multi_regions.bin

CC = gcc

## Flags
BINFLAG = -c
DEBUGFLAG = -g
OFLAG = -o
WALLFLAG = -Wall -Werror -pedantic
OPTIMZEFLAG = -O3
AIOFLAG = -lrt

LDFLAGS = $(AIOFLAG)
CFLAGS = $(BINFLAG) $(WALLFLAG) $(OPTIMIZEFLAG)

## Commands
RM = rm -fr
AR = ar -r
CP = cp
MKDIR = mkdir -p
