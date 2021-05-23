/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author minli
 * 
 * Modified : Iacovos G. Kolokasis
 * (01/04/19)
 */

package LogisticRegression.src.main.java;
import org.apache.log4j.Logger;
import org.apache.log4j.Level;
import java.util.Arrays;
import scala.Tuple2;
import org.apache.spark.api.java.*;
import org.apache.spark.api.java.function.Function;
import org.apache.spark.mllib.classification.LogisticRegressionModel;
import org.apache.spark.mllib.classification.LogisticRegressionWithSGD;
import org.apache.spark.mllib.linalg.Vectors;
import org.apache.spark.SparkConf;
import org.apache.spark.mllib.regression.LabeledPoint;
import org.apache.spark.rdd.RDD;
import org.apache.log4j.Logger;
import org.apache.log4j.Level;
import org.apache.spark.storage.StorageLevel;

public class LogisticRegressionApp {

    public static void main(String[] args) {
        if (args.length < 3) {
            System.out.println("usage: <input> <output>  <maxIterations> <StorageLevel> ");
            System.exit(0);
        }

        Logger.getLogger("org.apache.spark").setLevel(Level.WARN);
        Logger.getLogger("org.eclipse.jetty.server").setLevel(Level.OFF);

        String input = args[0];                         // Input File
        String output = args[1];                        // Output File
        int numIterations = Integer.parseInt(args[2]);  // Number of Iterations
        System.out.println("NumIteration," + numIterations);
        String  sl = args[3];                           // Storage Level
        int numPartitions = Integer.parseInt(args[4]);  // Number of Partitions

        // Initialize Storage Level
        StorageLevel level = StorageLevel.MEMORY_AND_DISK_SER();

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

        SparkConf conf = new SparkConf().setAppName("LogisticRegressionApp Example");
        JavaSparkContext sc = new JavaSparkContext(conf);
        conf.registerKryoClasses(new Class<?>[]{ LogisticRegressionApp.class});

        // Load and Parse Data
        long start = System.currentTimeMillis();

        // JavaRDD<String> data = sc.textFile(input, numPartitions).coalesce(numPartitions).persist(level);
        JavaRDD<String> data = sc.textFile(input, numPartitions).coalesce(numPartitions).persist(level);
        
        double loadTime = (double)(System.currentTimeMillis() - start) / 1000.0;

        // Parse Data
        start = System.currentTimeMillis();

        JavaRDD<LabeledPoint> parsedData = data.map(new Function<String, LabeledPoint>() { 
            public LabeledPoint call(String line) {
                return LabeledPoint.parse(line);
            }
        });
        
        RDD<LabeledPoint> parsedRDD_Data=JavaRDD.toRDD(parsedData).persist(level);

        double parseTime = (double)(System.currentTimeMillis() - start) / 1000.0;
        
        // Start Computation
        start = System.currentTimeMillis();
        
        // Building the model
        final LogisticRegressionModel model 
            = LogisticRegressionWithSGD.train(parsedRDD_Data, numIterations);

        // Evaluate model on training examples and compute training error
        JavaRDD<Tuple2<Double, Double>> valuesAndPreds = parsedData.map(
                new Function<LabeledPoint, Tuple2<Double, Double>>()
                {
                    public Tuple2<Double, Double> call(LabeledPoint point) { 
                        double prediction = model.predict(point.features());
                        return new Tuple2<Double, Double>(prediction, point.label());
                    }
                });

        // JavaRDD<Object>
        Double MSE = new JavaDoubleRDD(valuesAndPreds.map(
                    new Function<Tuple2<Double, Double>, Object>() {
                        public Object call(Tuple2<Double, Double> pair) {
                            return Math.pow(pair._1() - pair._2(), 2.0);
                        }
                    }).rdd()).mean();

        double computationTime = (double)(System.currentTimeMillis() - start) / 1000.0;

        System.out.println("LoadTime," + loadTime);
        System.out.println("ParseTime," + parseTime);
        System.out.println("ComputationTime," + computationTime);
        System.out.println("training Mean Squared Error = " + MSE);        

        sc.stop();
    }
}
