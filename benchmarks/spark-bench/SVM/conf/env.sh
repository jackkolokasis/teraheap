# for prepare #600M example=40G
################ 2GB #######################
#NUM_OF_EXAMPLES=10000
#NUM_OF_FEATURES=12500
#NUM_OF_PARTITIONS=128
#############################################

############## 18GB #######################
# NUM_OF_EXAMPLES=10000
# NUM_OF_FEATURES=100000     #18GB
###########################################

############## 32GB #######################
# NUM_OF_EXAMPLES=10000
# NUM_OF_FEATURES=200000     #36GB
# NUM_OF_PARTITIONS=72

############### 64GB #######################
#NUM_OF_EXAMPLES=10000
#NUM_OF_FEATURES=400000
#NUM_OF_PARTITIONS=128
############################################

############## 128GB #######################
NUM_OF_EXAMPLES=10000
NUM_OF_FEATURES=700000
NUM_OF_PARTITIONS=256
###########################################

################ 256GB #######################
#NUM_OF_EXAMPLES=20000
#NUM_OF_FEATURES=711108
#NUM_OF_PARTITIONS=256
#############################################

# for running
MAX_ITERATION=100

SPARK_STORAGE_MEMORYFRACTION=0.5    #0.79
