/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.spark.memory

import org.apache.spark.SparkConf
import org.apache.spark.storage.BlockId

/**
 * A [[MemoryManager]] that enforces a soft boundary between execution
 * and storage such that either side can borrow memory from the other.
 *
 * The region shared between execution and storage is a fraction of
 * (the total heap space - 300MB) configurable through
 * `spark.memory.fraction` (default 0.6). The position of the boundary
 * within this space is further determined by
 * `spark.memory.storageFraction` (default 0.5).  This means the size
 * of the storage region is 0.6 * 0.5 = 0.3 of the heap space by
 * default.
 *
 * Storage can borrow as much execution memory as is free until
 * execution reclaims its space.  When this happens, cached blocks
 * will be evicted from memory until sufficient borrowed memory is
 * released to satisfy the execution memory request.
 *
 * Similarly, execution can borrow as much storage memory as is free.
 * However, execution memory is *never* evicted by storage due to the
 * complexities involved in implementing this.  The implication is
 * that attempts to cache blocks may fail if execution has already
 * eaten up most of the storage space, in which case the new blocks
 * will be evicted immediately according to their respective storage
 * levels.
 *
 * @param onHeapStorageRegionSize Size of the storage region, in bytes.
 *                          This region is not statically reserved; execution can borrow from
 *                          it if necessary. Cached blocks can be evicted only if actual
 *                          storage memory usage exceeds this region.
 */
private[spark] class UnifiedMemoryManager private[memory] (
    conf: SparkConf,
    val maxHeapMemory: Long,
    onHeapStorageRegionSize: Long,
    numCores: Int)
  extends MemoryManager(
    conf,
    numCores,
    onHeapStorageRegionSize,
    maxHeapMemory - onHeapStorageRegionSize) {

  private def assertInvariants(): Unit = {
    assert(onHeapExecutionMemoryPool.poolSize + onHeapStorageMemoryPool.poolSize == maxHeapMemory)
    assert(
      offHeapExecutionMemoryPool.poolSize + offHeapStorageMemoryPool.poolSize == maxOffHeapMemory)
  }

  assertInvariants()

  override def maxOnHeapStorageMemory: Long = synchronized {
    maxHeapMemory - onHeapExecutionMemoryPool.memoryUsed
  }

  override def maxOffHeapStorageMemory: Long = synchronized {
    maxOffHeapMemory - offHeapExecutionMemoryPool.memoryUsed
  }

  /**
   * Try to acquire up to `numBytes` of execution memory for the current task and return the
   * number of bytes obtained, or 0 if none can be allocated.
   *
   * This call may block until there is enough free memory in some situations, to make sure each
   * task has a chance to ramp up to at least 1 / 2N of the total memory pool (where N is the # of
   * active tasks) before it is forced to spill. This can happen if the number of tasks increase
   * but an older task had a lot of memory already.
   */
  override private[memory] def acquireExecutionMemory(
      numBytes: Long,
      taskAttemptId: Long,
      memoryMode: MemoryMode): Long = synchronized {
    assertInvariants()
    assert(numBytes >= 0)
    val (executionPool, storagePool, storageRegionSize, maxMemory) = memoryMode match {
      case MemoryMode.ON_HEAP => (
        onHeapExecutionMemoryPool,
        onHeapStorageMemoryPool,
        onHeapStorageRegionSize,
        maxHeapMemory)
      case MemoryMode.OFF_HEAP => (
        offHeapExecutionMemoryPool,
        offHeapStorageMemoryPool,
        offHeapStorageMemory,
        maxOffHeapMemory)
    }

    /**
     * Grow the execution pool by evicting cached blocks, thereby
     * shrinking the storage pool.
     *
     * When acquiring memory for a task, the execution pool may need
     * to make multiple attempts. Each attempt must be able to evict
     * storage in case another task jumps in and caches a large block
     * between the attempts. This is called once per attempt.
     */
    def maybeGrowExecutionPool(extraMemoryNeeded: Long): Unit = {
      if (extraMemoryNeeded > 0) {
        // There is not enough free memory in the execution pool, so
        // try to reclaim memory from storage. We can reclaim any free
        // memory from the storage pool. If the storage pool has grown
        // to become larger than `storageRegionSize`, we can evict
        // blocks and reclaim the memory that storage has borrowed
        // from execution.
        val memoryReclaimableFromStorage = math.max(
          storagePool.memoryFree,
          storagePool.poolSize - storageRegionSize)
        // JK: The above storagePool.poolSize may be larger than the
        // initial maximum memory size of the Storage memory area,
        // mainly by borrowing the memory of the Execution memory
        // area.

        if (memoryReclaimableFromStorage > 0) {
          // Only reclaim as much space as is necessary and available:
          // JK: Shrink off the available memory in the Storage memory
          // area.
          val spaceToReclaim = storagePool.freeSpaceToShrinkPool(
            math.min(extraMemoryNeeded, memoryReclaimableFromStorage))
          storagePool.decrementPoolSize(spaceToReclaim)
          executionPool.incrementPoolSize(spaceToReclaim)
        }
      }
    }

    /**
     * The size the execution pool would have after evicting storage memory.
     *
     * The execution memory pool divides this quantity among the active tasks evenly to cap
     * the execution memory allocation for each task. It is important to keep this greater
     * than the execution pool size, which doesn't take into account potential memory that
     * could be freed by evicting storage. Otherwise we may hit SPARK-12155.
     *
     * Additionally, this quantity should be kept below `maxMemory` to arbitrate fairness
     * in execution memory allocation across tasks, Otherwise, a task may occupy more than
     * its fair share of execution memory, mistakenly thinking that other tasks can acquire
     * the portion of storage memory that cannot be evicted.
     */
    def computeMaxExecutionPoolSize(): Long = {
      maxMemory - math.min(storagePool.memoryUsed, storageRegionSize)
    }

    // JK: The parameters of the method require 2 functions:
    // maybeGrowExecutionPool: is used to control how to increase the
    //                         size of the Pool corresponding to the
    //                         Execution memory area
    //
    // computMaxExecutionPoolSize: is used obtain the size of the
    //                             current Execution memory area
    //                             corresponding to the Pool.
    executionPool.acquireMemory(
      numBytes, taskAttemptId, maybeGrowExecutionPool, () => computeMaxExecutionPoolSize)
  }

  override def acquireStorageMemory(
      blockId: BlockId,
      numBytes: Long,
      memoryMode: MemoryMode): Boolean = synchronized {
        // JK: Request numBytes size memory for BlockId
    assertInvariants()
    assert(numBytes >= 0)
    val (executionPool, storagePool, maxMemory) = memoryMode match {
      case MemoryMode.ON_HEAP => (
        onHeapExecutionMemoryPool,
        onHeapStorageMemoryPool,
        maxOnHeapStorageMemory)
      case MemoryMode.OFF_HEAP => (
        offHeapExecutionMemoryPool,
        offHeapStorageMemoryPool,
        maxOffHeapStorageMemory)
    }

    // JK: If the requested memory is larger than the maximum amount of
    // Storage memory (corresponding to the memory size returned by
    // the above method maxOnHeapStorageMemory()), the application
    // fails.
    if (numBytes > maxMemory) {
      // Fail fast if the block simply won't fit
      logInfo(s"Will not store $blockId as the required space ($numBytes bytes) exceeds our " +
        s"memory limit ($maxMemory bytes)")
      return false
    }

    // JK: If there is not enough memory in the Storage memory block
    // for the blockId to use, calculate how much memory is missing
    // from the current Storage memory area and the borrow from the
    // Execution Memory area
    if (numBytes > storagePool.memoryFree) {
      // There is not enough free memory in the storage pool, so try to borrow free memory from
      // the execution pool.
      val memoryBorrowedFromExecution = Math.min(executionPool.memoryFree,
        numBytes - storagePool.memoryFree)
      executionPool.decrementPoolSize(memoryBorrowedFromExecution)
      storagePool.incrementPoolSize(memoryBorrowedFromExecution)
    }

    // JK: If the Storage memory area can allocate memory for the BlockId,
    // it will be succesfully allocated directly;
    // if the memory borrowed from the execution memory area can
    // satisfy the blockId, the allocation is succesfully,
    // and if it cannot be satisfied, the allocation fails.
    storagePool.acquireMemory(blockId, numBytes)
  }

  // JK: Unroll memory, which is Storage memory
  // Unroll memory, used to unroll (expand) the specified block data
  // in Storage memory
  override def acquireUnrollMemory(
      blockId: BlockId,
      numBytes: Long,
      memoryMode: MemoryMode): Boolean = synchronized {
    acquireStorageMemory(blockId, numBytes, memoryMode)
  }
}

object UnifiedMemoryManager {

  // Set aside a fixed amount of memory for non-storage, non-execution purposes.
  // This serves a function similar to `spark.memory.fraction`, but guarantees that we reserve
  // sufficient memory for the system even for small heaps. E.g. if we have a 1GB JVM, then
  // the memory used for execution and storage will be (1024 - 300) * 0.6 = 434MB by default.
  private val RESERVED_SYSTEM_MEMORY_BYTES = 300 * 1024 * 1024

  def apply(conf: SparkConf, numCores: Int): UnifiedMemoryManager = {
    // JK: Get the maximum memory shared by the execution and storage
    // areas
    val maxMemory = getMaxMemory(conf)

    // JK: Construct a UnifiedMemoryManager object
    new UnifiedMemoryManager(
      conf,
      maxHeapMemory = maxMemory,
      // JK: The memory area size of the storage area is initially the
      // spark.memory.storageFraction of the maximum memory shared by
      // the execution and storage areas. The default is 0.5 which is
      // half
      onHeapStorageRegionSize =
        (maxMemory * conf.getDouble("spark.memory.storageFraction", 0.5)).toLong,
      numCores = numCores)
  }

  /**
   * Return the total amount of memory shared between execution and storage, in bytes.
   * Returns the maximum memory shared by the execution and storage
   * areas.
   */
  private def getMaxMemory(conf: SparkConf): Long = {
    // JK: Get he systems maximum memory systemMemory, take the parameter
    // spark.testing.memory, if not configured, take the maximumm
    // memory in the runtime enviroment
    val systemMemory = conf.getLong("spark.testing.memory", Runtime.getRuntime.maxMemory)

    // JK: Get the reseved memory. Take the parameter
    // spark.testing.reservedMemory
    // If not configured the default value is 300MB
    val reservedMemory = conf.getLong("spark.testing.reservedMemory",
      if (conf.contains("spark.testing")) 0 else RESERVED_SYSTEM_MEMORY_BYTES)

    // JK: Take the minimum system memory minSystemMemory
    // 1.5 * reservedMemory
    val minSystemMemory = (reservedMemory * 1.5).ceil.toLong
    if (systemMemory < minSystemMemory) {
      throw new IllegalArgumentException(s"System memory $systemMemory must " +
        s"be at least $minSystemMemory. Please increase heap size using the --driver-memory " +
        s"option or spark.driver.memory in Spark configuration.")
    }
    // SPARK-12759 Check executor memory to fail fast if memory is insufficient
    if (conf.contains("spark.executor.memory")) {
      val executorMemory = conf.getSizeAsBytes("spark.executor.memory")
      if (executorMemory < minSystemMemory) {
        throw new IllegalArgumentException(s"Executor memory $executorMemory must be at least " +
          s"$minSystemMemory. Please increase executor memory using the " +
          s"--executor-memory option or spark.executor.memory in Spark configuration.")
      }
    }

    // JK: Calculate the available memory usableMemory, that is, the
    // systems maximum available memory - reserved memory
    val usableMemory = systemMemory - reservedMemory

    // JK: Take the proportion of available memory
    val memoryFraction = conf.getDouble("spark.memory.fraction", 0.6)

    // JK: The maximum memory shared by the returned execution and
    // storage areas is usableMemory * memoryFraction
    (usableMemory * memoryFraction).toLong
  }
}
