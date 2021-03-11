
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
LIBHEADERS =  $(INCLUDEDIR)/regions.h
LIBREGIONSOBJS = $(SRCDIR)/regions.o
REGIONSLIBRARY = $(LIBDIR)/libregions.so
APPOBJ = $(SRCDIR)/main.o
TEST1OBJ = $(TESTDIR)/test1.o
EXECUTABLE = regions.bin
TEST1EXE = test1.bin

CC = gcc

## Flags
BINFLAG = -c
DEBUGFLAG = -ggdb3
OFLAG = -o
WALLFLAG = -Wall -Werror
OPTIMZEFLAG = -O3

CFLAGS = $(BINFLAG) $(WALLFLAG) $(OPTIMIZEFLAG)

## Commands
RM = rm -fr
AR = ar -r
CP = cp
MKDIR = mkdir -p
