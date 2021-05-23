#
# makefile for Text_datagen 
#
#

all: gen_random_text.cpp pgen_random_text.cpp
	g++ -Wall gen_random_text.cpp -I/usr/local/include/gsl -lgsl -lgslcblas -fexceptions -o gen_random_text
	g++ -Wall pgen_random_text.cpp -I/usr/local/include/gsl -lgsl -lgslcblas -fexceptions -o pgen_random_text

.PHONY : clean
clean :
	-rm gen_random_text
	-rm pgen_random_text
