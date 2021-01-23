#!/usr/bin/env bash

nvim -p /opt/spark/spark-2.3.0-kolokasis/conf/spark-env.sh \
    /opt/spark/spark-2.3.0-kolokasis/conf/spark-defaults.conf \
    /home1/public/kolokasis/sparkPersistentMemory/benchmarks/spark-bench-legacy_sith6/conf/env.sh \
    /home1/public/kolokasis/sparkPersistentMemory/benchmarks/spark-bench-legacy_sith6/KMeans/conf/env.sh \
    /home1/public/kolokasis/sparkPersistentMemory/benchmarks/spark-bench-legacy_sith6/LinearRegression/conf/env.sh \
    /home1/public/kolokasis/sparkPersistentMemory/benchmarks/spark-bench-legacy_sith6/LogisticRegression/conf/env.sh \
    /home1/public/kolokasis/sparkPersistentMemory/benchmarks/scripts/ser_br.sh \
    /home1/public/kolokasis/sparkPersistentMemory/benchmarks/scripts/mlExperiments_new.sh
