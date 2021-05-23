/*
 * (C) Copyright IBM Corp. 2015
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.spark.SparkConf;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function;
import org.apache.spark.mllib.clustering.KMeans;
import org.apache.spark.mllib.clustering.KMeansModel;
import org.apache.spark.mllib.linalg.Vector;
import org.apache.spark.mllib.linalg.Vectors;
import org.apache.spark.storage.StorageLevel;

public class KmeansAppJava {
  public static void main(String[] args) {
    Logger.getLogger("org.eclipse.jetty.server").setLevel(Level.OFF);
    Logger.getLogger("org.apache.spark").setLevel(Level.WARN);
    if (args.length < 6) {
      System.out.println("usage: <input> <output> <numClusters> <maxIterations> <runs> - optional <numPartitions>");
      System.exit(0);
    }
    String input = args[0];
    String output = args[1];
    int numClusters = Integer.parseInt(args[2]);
    int maxIterations = Integer.parseInt(args[3]);
    int runs = Integer.parseInt(args[4]);
    String sl = args[5];
    int numPartitions = Integer.parseInt(args[6]);
        
    StorageLevel sLevel = StorageLevel.MEMORY_AND_DISK_SER();

    switch (sl) { 
        case "MEMORY_AND_DISK":
            sLevel = StorageLevel.MEMORY_AND_DISK();
            break;

        case "MEMORY_ONLY":
            sLevel = StorageLevel.MEMORY_ONLY();
            break;

        case "OFF_HEAP":
            sLevel = StorageLevel.OFF_HEAP();
            break;

        case"PMEM_OFF_HEAP":
            sLevel = StorageLevel.PMEM_OFF_HEAP();
            break;
        
        case"DISK_ONLY":
            sLevel = StorageLevel.DISK_ONLY();
            break;
        
        case"NONE":
            sLevel = StorageLevel.NONE();
            break;
    }

    SparkConf conf = new SparkConf().setAppName("K-means Example");
    JavaSparkContext jsc = new JavaSparkContext(conf);

    long start = System.currentTimeMillis();
    
    JavaRDD<String> data = jsc.textFile(input).coalesce(numPartitions);
    //data.count();
    
    double loadTime = (double) (System.currentTimeMillis() - start) / 1000.0;

    // Cluster the data into classes using KMeans
    start = System.currentTimeMillis();

    JavaRDD<Vector> parsedData = data.map(s -> {
        String[] sarray = s.split(" ");
        double[] values = new double[sarray.length];
        for (int i = 0; i < sarray.length; i++) {
            values[i] = Double.parseDouble(sarray[i]);
        }
        return Vectors.dense(values);
        });

    parsedData.persist(sLevel);

    //double loadTime = (double) (System.currentTimeMillis() - start) / 1000.0;

    // Cluster the data into classes using KMeans
    //start = System.currentTimeMillis();
    KMeansModel clusters = KMeans.train(parsedData.rdd(), numClusters, maxIterations, runs, KMeans.K_MEANS_PARALLEL(), 127L);
    double trainingTime = (double) (System.currentTimeMillis() - start) / 1000.0;

    // Evaluate clustering by computing Within Set Sum of Squared Errors
    start = System.currentTimeMillis();
    double WSSSE = clusters.computeCost(parsedData.rdd());
    double testTime = (double) (System.currentTimeMillis() - start) / 1000.0;

    // Save Model
    // start = System.currentTimeMillis();
    // clusters.save(jsc.sc(), output);
    // double saveTime = (double) (System.currentTimeMillis() - start) / 1000.0;
    //
    
    double saveTime = 0;

    System.out.println("LoadTime," + loadTime);
    System.out.println("ComputationTime," + (trainingTime + testTime));
    System.out.println("SaveTime," + saveTime);
    System.out.println("Within Set Sum of Squared Errors = " + WSSSE);

    jsc.stop();
  }
}
