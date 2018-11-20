/**
 * Illustrates flatMap + countByValue for wordcount.
 */
package org.apache.spark.examples

import org.apache.spark._
import org.apache.spark.storage._
import org.apache.spark.SparkContext._

object WordCount {
    def main(args: Array[String]) {
      val startTime = System.currentTimeMillis()
      val inputFile = args(0)
      val output = args(1)
      var storageLevel: StorageLevel = StorageLevel.NONE

      if (args(2) == "onHeap")
        storageLevel = StorageLevel.MEMORY_ONLY
      else if (args(2) == "offHeap")
        storageLevel = StorageLevel.OFF_HEAP
      else if (args(2) == "disk")
        storageLevel = StorageLevel.DISK_ONLY
      else 
        storageLevel = StorageLevel.PMEM_OFF_HEAP
      
      val conf = new SparkConf().setAppName("wordCount")
      
      // Create a Scala Spark Context.
      val sc = new SparkContext(conf)

      // Load our input data.
      val input =  sc.textFile(inputFile).persist(storageLevel)
      println("Storage Level: Memory = " + input.getStorageLevel.useMemory)
      println("Storage Level: Disk = " + input.getStorageLevel.useDisk)
      println("Storage Level: OffHeap = " + input.getStorageLevel.useOffHeap)

      // Split up into words.
      val words = input.flatMap(line => line.split(" "))
     
      // Transform into word and count.
      val counts = words.map(word => (word, 1)).reduceByKey{case (x, y) => x + y}
      
      // Save the word count back out to a text file, causing evaluation.
      counts.saveAsTextFile(output)
     
      val stopTime = System.currentTimeMillis()
      println("Execution Time: " + (stopTime - startTime)/1000)
    }
}
