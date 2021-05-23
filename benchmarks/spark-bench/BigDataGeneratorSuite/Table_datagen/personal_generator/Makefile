#
# makefile for gen_personal_resume 
#
#

all: gen_personal_resume.cpp pgen_personal_resume.cpp
	g++ -Wall gen_personal_resume.cpp -I/usr/local/include/gsl -lgsl -lgslcblas -fexceptions -o gen_personal_resume
	g++ -Wall pgen_personal_resume.cpp -I/usr/local/include/gsl -lgsl -lgslcblas -fexceptions -o pgen_personal_resume


.PHONY : clean
clean :
	-rm gen_personal_resume
	-rm pgen_personal_resume
