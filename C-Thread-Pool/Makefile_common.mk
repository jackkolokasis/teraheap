
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
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include

## Depended files
LIBHEADERS =  $(INCLUDEDIR)/thpool.h
LIBTPOOLOBJS = $(SRCDIR)/thpool.o
TPOOLLIBRARY = $(LIBDIR)/libthreadpool.so

CC = gcc

## Flags
BINFLAG = -c
DEBUGFLAG = -ggdb3
OFLAG = -o
WALLFLAG = -Wall -Werror -pedantic
OPTIMZEFLAG = -O3
PTHREADFLAG = -pthread

LDFLAGS = $(PTHREADFLAG)
CFLAGS = $(BINFLAG) $(WALLFLAG) $(OPTIMZEFLAG)

## Commands
RM = rm -fr
AR = ar -r
CP = cp
MKDIR = mkdir -p
