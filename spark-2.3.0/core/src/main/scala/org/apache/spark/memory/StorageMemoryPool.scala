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
// scalastyle:off

package org.apache.spark.memory

import javax.annotation.concurrent.GuardedBy

import org.apache.spark.internal.Logging
import org.apache.spark.storage.BlockId
import org.apache.spark.storage.memory.MemoryStore

/**
 * Performs bookkeeping for managing an adjustable-size pool of memory that is used for storage
 * (caching).
 *
 * @param lock a [[MemoryManager]] instance to synchronize on
 * @param memoryMode the type of memory tracked by this pool (on- or off-heap)
 */
private[memory] class StorageMemoryPool(
    lock: Object,
    memoryMode: MemoryMode
  ) extends MemoryPool(lock) with Logging {

    /**
     * Jack Kolokasis (05/10/18)
     *
     * Based on the memorymode type we set the poolname 
     */ 
  private[this] val poolName: String = memoryMode match {
    case MemoryMode.ON_HEAP => "on-heap storage"
    case MemoryMode.OFF_HEAP => "off-heap storage"
    case MemoryMode.PMEM_OFF_HEAP => "pmem off-heap storage"
  }

  println("MemoryManager::StorageMemoryPool::poolname::" + poolName)

  @GuardedBy("lock")
  private[this] var _memoryUsed: Long = 0L

  override def memoryUsed: Long = lock.synchronized {
    println("StorageMemoryPool::memoryUsed")
    _memoryUsed
  }

  private var _memoryStore: MemoryStore = _
  def memoryStore: MemoryStore = {
    println("StorageMemoryPool::memoryStore")
    if (_memoryStore == null) {
      throw new IllegalStateException("memory store not initialized yet")
    }
    _memoryStore
  }

  /**
   * Set the [[MemoryStore]] used by this manager to evict cached blocks.
   * This must be set after construction due to initialization ordering constraints.
   */
  final def setMemoryStore(store: MemoryStore): Unit = {
    println("StorageMemoryPool::setMemoryStore")
    _memoryStore = store
  }

  /**
   * Acquire N bytes of memory to cache the given block, evicting existing ones if necessary.
   *
   * @return whether all N bytes were successfully granted.
   */
  def acquireMemory(blockId: BlockId, numBytes: Long): Boolean = lock.synchronized {
    println("StorageMemoryPool::acquireMemory")
    val numBytesToFree = math.max(0, numBytes - memoryFree)
    acquireMemory(blockId, numBytes, numBytesToFree)
  }

  /**
   * Acquire N bytes of storage memory for the given block, evicting existing ones if necessary.
   *
   * @param blockId the ID of the block we are acquiring storage memory for
   * @param numBytesToAcquire the size of this block
   * @param numBytesToFree the amount of space to be freed through evicting blocks
   * @return whether all N bytes were successfully granted.
   */
  def acquireMemory(
      blockId: BlockId,
      numBytesToAcquire: Long,
      numBytesToFree: Long): Boolean = lock.synchronized {
    println("StorageMemoryPool::acquireMemory")
    assert(numBytesToAcquire >= 0)
    assert(numBytesToFree >= 0)
    assert(memoryUsed <= poolSize)
    if (numBytesToFree > 0) {
      memoryStore.evictBlocksToFreeSpace(Some(blockId), numBytesToFree, memoryMode)
    }

    /**
     * NOTE: If the memory store evicts blocks, then those evictions will synchronously call
     * back into this StorageMemoryPool in order to free memory. Therefore, these variables
     * should have been updated.
     */
    val enoughMemory = numBytesToAcquire <= memoryFree
    if (enoughMemory) {
      _memoryUsed += numBytesToAcquire
    }
    enoughMemory
  }

  // JK: Freeing Storage Memory
  def releaseMemory(size: Long): Unit = lock.synchronized {
    println("StorageMemoryPool::releaseMemory")
    if (size > _memoryUsed) {
      logWarning(s"Attempted to release $size bytes of storage " +
        s"memory when we only have ${_memoryUsed} bytes")
      _memoryUsed = 0
    } else {
      _memoryUsed -= size
    }
  }

  def releaseAllMemory(): Unit = lock.synchronized {
    println("StorageMemoryPool::releaseAllMemory")
    _memoryUsed = 0
  }

  /**
   * Free space to shrink the size of this storage memory pool by `spaceToFree` bytes.  Note: this
   * method doesn't actually reduce the pool size but relies on the caller to do so.
   *
   * @return number of bytes to be removed from the pool's capacity.
   */
  def freeSpaceToShrinkPool(spaceToFree: Long): Long = lock.synchronized {
    println("StorageMemoryPool::freeSpaceToShrinkPool::spaceToFree = " + spaceToFree)
    val spaceFreedByReleasingUnusedMemory = math.min(spaceToFree, memoryFree)
    /**
     * Storage memory area needs to release the memory of remainingSpaceToFree size.
     */
    val remainingSpaceToFree = spaceToFree - spaceFreedByReleasingUnusedMemory

    /**
     * Greater than 0 means that there is no available memory in the current Storage memory area.
     * You need to clear the block in the Storage memory area to implement Shrink operation.
     */
    if (remainingSpaceToFree > 0) {
      /** If reclaiming free memory did not adequately shrink the pool, begin evicting blocks: */
      val spaceFreedByEviction = 
        memoryStore.evictBlocksToFreeSpace(None, remainingSpaceToFree, memoryMode)
      
      /**
       * When a block is released, BlockManager.dropFromMemory() calls releaseMemory(), so we do not
       * need to decrement _memoryUsed here. However, we do need to decrement the pool size.
       */
      spaceFreedByReleasingUnusedMemory + spaceFreedByEviction
    } else {
      spaceFreedByReleasingUnusedMemory
    }
  }
}
// scalastyle:on
