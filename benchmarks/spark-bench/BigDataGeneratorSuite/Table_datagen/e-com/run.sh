#!/bin/bash

date
#java -XX:NewRatio=1 -jar pdgf.jar -load personal_resume-schema.xml -load personal_resume-generation.xml -closeWhenDone -workers 1 -sf 1000 -start
java -XX:NewRatio=1 -jar pdgf.jar -load e-com-schema.xml -load e-com-generation.xml -closeWhenDone -workers 1 -sf 1000 -start

date
