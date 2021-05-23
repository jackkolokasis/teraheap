# for gen_data.sh;  200M data size = 1 million points

############## 32GB #######################
NUM_OF_EXAMPLES=20000
NUM_OF_FEATURES=88888
NUM_OF_PARTITIONS=128
###########################################

############## 64GB #######################
#NUM_OF_EXAMPLES=20000
#NUM_OF_FEATURES=177777
#NUM_OF_PARTITIONS=128
###########################################

# for run.sh
NUM_OF_CLASS_C=2
impurityC="gini"
maxDepthC=2
maxBinsC=5
modeC="Classification"
#${NUM_OF_CLASS_C} ${impurityC} ${maxDepthC} ${maxBinsC} ${modeC}

NUM_OF_CLASS_R=2
impurityR="variance"
maxDepthR=2
maxBinsR=5
modeR="Regression"
#${NUM_OF_CLASS_R} ${impurityR} ${maxDepthR} ${maxBinsR} ${modeR}

MAX_ITERATION=3

SPARK_STORAGE_MEMORYFRACTION=0.79
