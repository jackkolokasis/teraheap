###################################################
#
# file: Makefile
#
# @Author:   Iacovos G. Kolokasis
# @Version:  09-10-2018
# @email:    kolokasis@ics.forth.gr
#
# Makefile build the example for the test
#
####################################################

JAVAC		= javac
SCALAC		= scalac
JAVA		= java
SCALA		= scala
JAR			= TeraCache.jar

#TARGET		= Test
TARGET		= AllocateClass
CLASSPATH 	= ".:../TeraCache.jar"

all: $(TARGET).class
	$(SCALA) -cp $(CLASSPATH) $(TARGET)

$(TARGET).class: $(TARGET).scala
	$(SCALAC) -cp $(CLASSPATH) $<

clean:
	rm -rf $(TARGET).class
