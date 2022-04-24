# The parameters for data generation. 100 million points roughly produces 36GB data size
#NUM_OF_POINTS=10000
#NUM_OF_CLUSTERS=4
#DIMENSIONS=112500  #< 18GB
#DIMENSIONS=200000 #< 32GB
#DIMENSIONS=400000 #< 64G
#DIMENSIONS=800000 #< 140G
#NUM_OF_PARTITIONS=100

################# 2GB #######################
#NUM_OF_POINTS=10000
#DIMENSIONS=12500
#NUM_OF_CLUSTERS=4
#NUM_OF_PARTITIONS=128
#############################################

################ 64GB #######################
#NUM_OF_POINTS=10000
#DIMENSIONS=400000
#NUM_OF_CLUSTERS=4
#NUM_OF_PARTITIONS=128
############################################

############### 128GB #######################
NUM_OF_POINTS=10000
DIMENSIONS=800000
NUM_OF_CLUSTERS=4
NUM_OF_PARTITIONS=256
############################################

SCALING=0.6
MAX_ITERATION=100
NUM_RUN=1

SPARK_STORAGE_MEMORYFRACTION=0.8
