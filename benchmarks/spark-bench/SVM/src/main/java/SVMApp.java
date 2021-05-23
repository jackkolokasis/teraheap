/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author minli
 */
package SVM.src.main.java;
import org.apache.log4j.Logger;
import org.apache.log4j.Level;
import java.util.Arrays;
import java.util.Random;

import scala.Tuple2;

import org.apache.spark.api.java.*;
import org.apache.spark.api.java.function.Function;
import org.apache.spark.mllib.classification.*;
import org.apache.spark.mllib.evaluation.BinaryClassificationMetrics;
import org.apache.spark.mllib.linalg.Vector;
import org.apache.spark.mllib.regression.LabeledPoint;
import org.apache.spark.mllib.util.MLUtils;
import org.apache.spark.SparkConf;
import org.apache.spark.SparkContext;
import org.apache.log4j.Logger;
import org.apache.log4j.Level;
import org.apache.spark.storage.StorageLevel;
import org.apache.spark.rdd.RDD;

//static class Class1 {}
 // static class Class2 {}
  
public class SVMApp {
    public static void main(String[] args) {
        if (args.length < 3) {
            System.out.println("usage: <input> <output>  <maxIterations> <StorageLevel>");
            System.exit(0);
        }
        Logger.getLogger("org.apache.spark").setLevel(Level.WARN);
        Logger.getLogger("org.eclipse.jetty.server").setLevel(Level.OFF);

        String input = args[0];                         // Input File
        String output = args[1];                        // Output File
        int numIterations = Integer.parseInt(args[2]);  // Number of Iterations
        String sl = args[3];                            // Storage Level
        int numPartitions = Integer.parseInt(args[4]);  // Number of Partitions
        
        // Default Storage Level 
        StorageLevel level = StorageLevel.MEMORY_AND_DISK_SER();

        // Select Storage Level to persist RDDs
        switch (sl) { 
            case "MEMORY_AND_DISK":
                level = StorageLevel.MEMORY_AND_DISK();
                break;

            case "MEMORY_ONLY":
                level = StorageLevel.MEMORY_ONLY();
                break;

            case "OFF_HEAP":
                level = StorageLevel.OFF_HEAP();
                break;
                
            case"PMEM_OFF_HEAP":
                level = StorageLevel.PMEM_OFF_HEAP();
                break;
            
            case"DISK_ONLY":
                level = StorageLevel.DISK_ONLY();
                break;
            
            case"NONE":
                level = StorageLevel.NONE();
                break;
        }

        // Set Spark Configuration Parameter
        SparkConf conf = new SparkConf().setAppName("SVM Classifier Example");
        JavaSparkContext sc = new JavaSparkContext(conf);

        // Load training data in LIBSVM format
        long start = System.currentTimeMillis();

        JavaRDD<String> data = sc.textFile(input, numPartitions).coalesce(numPartitions).persist(level);
        // data.count();
        
        double loadTime = (double)(System.currentTimeMillis() - start) / 1000.0;
        
        // Computation Time
        start = System.currentTimeMillis();

        JavaRDD<LabeledPoint> parsedData  = data.map(
                new Function<String, LabeledPoint>() {
                    public LabeledPoint call(String line) {
                        return LabeledPoint.parse(line);
                    }
                });
        
        RDD<LabeledPoint> parsedRDD_Data=JavaRDD.toRDD(parsedData).persist(level);

        // Split data into two taining [60% training data, 40% testing data].
        JavaRDD<LabeledPoint> training = parsedData.sample(false, 0.6, 11L);
        training.persist(level);			
        JavaRDD<LabeledPoint> test = parsedData.subtract(training);

        // Run training algorithm to build the model
        final SVMModel model = SVMWithSGD.train(training.rdd(), numIterations);

        // Clear the default threshold.
        model.clearThreshold();

        // Compute raw scores on the test set
        JavaRDD<Tuple2<Object, Object>> scoreAndLabels = test.map(p ->
                new Tuple2<>(model.predict(p.features()), p.label()));

        // Get Evaluation metrics
        BinaryClassificationMetrics metrics = 
            new BinaryClassificationMetrics(JavaRDD.toRDD(scoreAndLabels));
        double auROC = metrics.areaUnderROC();

        System.out.println("Area Under ROC = " + auROC);

        double computationTime = (double)(System.currentTimeMillis() - start) / 1000.0;

        System.out.println("LoadTime," + loadTime);
        System.out.println("ComputationTime," + computationTime);

        sc.stop();
    }
}
