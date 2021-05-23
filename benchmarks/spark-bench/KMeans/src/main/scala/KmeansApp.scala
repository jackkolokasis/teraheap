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

/**************************************************
*
* file: KmeansApp.scala
* 
* @Author:   Iacovos G. Kolokasis
* @Version:  26-03-2019
* @email:    kolokasis@ics.forth.gr
* 
* KMeans in scala implementation
* 
**************************************************
*/

// scalastyle:off println
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
import org.apache.spark.{ SparkContext, SparkConf}

object KMeansApp {
    def main(args: Array[String]) {
     Logger.getLogger("org.apache.spark").setLevel(Level.WARN);
     Logger.getLogger("org.eclipse.jetty.server").setLevel(Level.OFF);
    
     if (args.length < 5) {
         println("usage:" +
             " <input> <output> <numClusters> <maxIterations> <runs> - optional <StorageLevel>")
         System.exit(0)
     }

     val conf = new SparkConf
     conf.setAppName("Spark ConnectedComponent Application")
     val sc = new SparkContext(conf)

     val input = args(0) 
     val output = args(1)
     val numClusters = args(2).toInt
     val numIteration = args(3).toInt
     val runs = args(4)
     val storageLevel = args(5)

     //var sl: StorageLevel = StorageLevel.MEMORY_AND_DISK;
     var sl: StorageLevel = StorageLevel.NONE
     
     //if(storageLevel == "MEMORY_ONLY")
     //  sl = StorageLevel.MEMORY_ONLY
     //else if(storageLevel == "OFF_HEAP")
     //  sl = StorageLevel.OFF_HEAP
     //else if(storageLevel == "PMEM_OFF_HEAP")
     //  sl = StorageLevel.PMEM_OFF_HEAP
     //else if(storageLevel == "MEMORY_AND_DISK")
     //  sl = StorageLevel.MEMORY_AND_DISK

     // Load and parse data
     val data = sc.textFile(input)
     val parsedData = data.map(s => Vectors.dense(s.split(' ').map(_.toDouble))).persist(sl)

     // Cluster the data into two classes using KMeans
     val clusters = KMeans.train(parsedData, numClusters, numIteration) 

     // Evaluate clustering by computing Within Set Sum of Squared Errors
     val WSSSE = clusters.computeCost(parsedData)
     println(s"Within Set Sum of Squared Errors = $WSSSE") 

     // Save model
     clusters.save(sc, output)

     sc.stop()
    }
}

// scalastyle:on println
