## Application parameters#32G date size=400 million examples; 1G= 12.5

################ 2GB #######################
#NUM_OF_EXAMPLES=20000
#NUM_OF_FEATURES=5555
#NUM_OF_PARTITIONS=128
#############################################

############## 18GB #######################
# NUM_OF_EXAMPLES=20000
# NUM_OF_FEATURES=50000
###########################################

############### 20GB #######################                                     
#NUM_OF_EXAMPLES=20000                                                           
#NUM_OF_FEATURES=55550                                                           
#NUM_OF_PARTITIONS=256                                                           
############################################

############## 32GB #######################
# NUM_OF_EXAMPLES=20000
# NUM_OF_FEATURES=88888
# NUM_OF_PARTITIONS=72
###########################################

############### 64GB #######################
NUM_OF_EXAMPLES=20000
NUM_OF_FEATURES=177777
NUM_OF_PARTITIONS=256
############################################

################## 128GB #######################
#NUM_OF_EXAMPLES=20000
#NUM_OF_FEATURES=355554
#NUM_OF_PARTITIONS=256
###############################################

################# 256GB #######################
#NUM_OF_EXAMPLES=20000
#NUM_OF_FEATURES=711108
#NUM_OF_PARTITIONS=256
##############################################

ProbOne=0.2
EPS=0.5

MAX_ITERATION=100
SPARK_STORAGE_MEMORYFRACTION=0.9
