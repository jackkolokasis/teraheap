
#--------------------------------------------------
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  19-06-2020
# @email:    kolokasis@ics.forth.gr
#
# Makefile to build TeraCache library
# This Makefile allow all targets to be self
# documenting.
#
#--------------------------------------------------

CC			  = g++
JAVAC		  = javac
JAVAH		  = javah

JAVA_PATH	  = /usr/lib/jvm/java-8-kolokasis/build/linux-x86_64-normal-server-release/jdk
CLASS_PATH    = com.teraCache
CLASSES       = classes

SRC_DIR	      = src/com/teraCache
JNI_DIR       = jni
LIB_DIR		  = lib
OBJ_DIR		  = obj

TARGET        = TeraCache
C_TARGET	  = $(JNI_DIR)/com_teraCache_$(TARGET).cpp
H_TARGET	  = $(JNI_DIR)/com_teraCache_$(TARGET).h
LIB_TARGET	  = $(LIB_DIR)/lib$(TARGET).so
JAR_TARGET	  = $(TARGET).jar

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

help: 		## Show make help
	@fgrep -h "##" $(MAKEFILE_LIST) | fgrep -v fgrep | sed -e 's/\\$$//' | sed -e 's/##//'


all: 		## Build all shared libraries and classes
all: $(LIB_TARGET)

$(LIB_TARGET): $(OBJ_DIR)/$(TARGET).o 
	@mkdir -p $(LIB_DIR)
	$(CC) -shared $< -o $@


$(OBJ_DIR)/$(TARGET).o: $(TARGET).h
	@mkdir -p $(OBJ_DIR)
	$(CC) -fpic -I"$(JAVA_PATH)/include/" -I"$(JAVA_PATH)/include/linux/" -c $(C_TARGET) -o $@ 


$(TARGET).h: $(TARGET).class
	$(JAVAH) -d $(JNI_DIR) -cp $(CLASSES):/usr/share/scala/lib/scala-library.jar $(CLASS_PATH).$*


$(TARGET).class: 
	@mkdir -p $(CLASSES)
	$(JAVAC) -d $(CLASSES) -cp ".:/usr/share/scala/lib/scala-library.jar" $(SRC_DIR)/$(TARGET).java


$(JAR_TARGET):
	cd $(CLASSES) && \
	echo Manifest-Version: 1.0 > MANIFEST.MF && \
	echo Created-By: Iacovos G. Kolokasis >> MANIFEST.MF &&\
	echo Class-Path: com >> MANIFEST.MF &&\
	jar cvfm $@ MANIFEST.MF com/teraCache/TeraCache.class &&\
	rm MANIFEST.MF &&\
	mv $@ ../ &&\
	cd - &&\
	chmod +x $@

install:	## Install the shared libraries (*.so)
	install -D $(LIB_TARGET) $(PREFIX)/lib/
	install -D $(LIB_TARGET) /usr/lib/
	install -D $(H_TARGET) $(PREFIX)/include/


uninstall:	## Uninstall the shared libraries (*.so)
	rm $(PREFIX)/$(LIB_TARGET)
	rm /usr/$(LIB_TARGET)
	rm $(PREFIX)/include/$(notdir $(H_TARGET))

clean:		## Delete all intermediate files
	rm -rf $(H_TARGET) $(OBJ_DIR) 

distclean:	## Delete all executable files (*.class, etc)
	rm -rf $(LIB_DIR) $(JAR_TARGET) $(CLASSES)
	$(MAKE) clean -C tests

test:		## Test the TeraCache.jar library works properly
	$(MAKE) all -C tests
