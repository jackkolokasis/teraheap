
#--------------------------------------------------
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-08-2018
# @email:    kolokasis@ics.forth.gr
#
# Makefile Build libNVMUnsafe library
# This Makefile allow all targets to be self
# documenting.
#
#--------------------------------------------------

CC			  = gcc
JAVAC		  = javac
JAVAH		  = javah

JAVA_PATH	  = /usr/lib/jvm/java-8-oracle
CLASS_PATH    = com.nvmUnsafe
LIB_PATH  	  = libpmemmalloc/lib
CLASSES       = classes

SRC_DIR	      = src/com/nvmUnsafe
JNI_DIR       = jni
LIB_DIR		  = lib
OBJ_DIR		  = obj

TARGET        = NVMUnsafe
C_TARGET	  = $(JNI_DIR)/com_nvmUnsafe_$(TARGET).c
H_TARGET	  = $(JNI_DIR)/com_nvmUnsafe_$(TARGET).h
LIB_TARGET	  = $(LIB_DIR)/lib$(TARGET).so
JAR_TARGET	  = $(TARGET).jar

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

help: 		## Show make help
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##//'


all: 		## Build all shared libraries and classes
all: pmemalloc $(LIB_TARGET)


pmemalloc:
	$(MAKE) -C libpmemmalloc


$(LIB_TARGET): $(OBJ_DIR)/$(TARGET).o 
	@mkdir -p $(LIB_DIR)
	$(CC) -shared $< -Llibpmemmalloc/lib/ -lpmemalloc -lpmem -o $@


$(OBJ_DIR)/$(TARGET).o: $(TARGET).h
	@mkdir -p $(OBJ_DIR)
	$(CC) -fpic -I"$(JAVA_PATH)/include/" -I"$(JAVA_PATH)/include/linux/" -c $(C_TARGET) -o $@ 


$(TARGET).h: $(TARGET).class
	$(JAVAH) -d $(JNI_DIR) -cp $(CLASSES) $(CLASS_PATH).$*


$(TARGET).class: 
	@mkdir -p $(CLASSES)
	$(JAVAC) -d $(CLASSES) $(SRC_DIR)/$(TARGET).java


$(JAR_TARGET):
	echo Manifest-Version: 1.0 > MANIFEST.MF
	echo Class-Path: classes/ >> MANIFEST.MF
	jar cvfm $@ MANIFEST.MF $(CLASSES)/
	rm MANIFEST.MF
	chmod +x $@


install:	## Install the shared libraries (*.so)
	install -D $(LIB_TARGET) $(PREFIX)/lib/
	install -D $(H_TARGET) $(PREFIX)/include/
	install -D $(LIB_PATH)/libpmemalloc.so $(PREFIX)/lib


clean:		## Delete all intermediate files
	rm -rf $(H_TARGET) $(OBJ_DIR) 


distclean:	## Delete all executable files (*.class, etc)
	rm -rf $(LIB_DIR) $(JAR_TARGET) $(CLASSES)
	$(MAKE) distclean -C libpmemmalloc
