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

package org.apache.spark.unsafe;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;


import sun.misc.Cleaner;
import sun.misc.Unsafe;

import com.nvmUnsafe.*;

public final class Platform {

  private static final Unsafe _UNSAFE;

  public static final int BOOLEAN_ARRAY_OFFSET;

  public static final int BYTE_ARRAY_OFFSET;

  public static final int SHORT_ARRAY_OFFSET;

  public static final int INT_ARRAY_OFFSET;

  public static final int LONG_ARRAY_OFFSET;

  public static final int FLOAT_ARRAY_OFFSET;

  public static final int DOUBLE_ARRAY_OFFSET;

  private static final boolean unaligned;


  /* Non-Volatile Unsafe Object         */
  private static final NVMUnsafe _NVM_UNSAFE;

  /* Non-Volatile address allocation    */
  private static final long PMEM_ADDR;
 
  // TODO:
  // This fields are might for sql support. 
  // At this time is not urgent to be developed
  //  public static final int PMEM_BOOLEAN_ARRAY_OFFSET;
  //
  //  public static final int PMEM_BYTE_ARRAY_OFFSET;
  //
  //  public static final int PMEM_SHORT_ARRAY_OFFSET;
  //
  //  public static final int PMEM_INT_ARRAY_OFFSET;
  //
  //  public static final int PMEM_LONG_ARRAY_OFFSET;
  //
  //  public static final int PMEM_FLOAT_ARRAY_OFFSET;
  //
  //  public static final int PMEM_DOUBLE_ARRAY_OFFSET;
  //

  static {
    boolean _unaligned;
    String arch = System.getProperty("os.arch", "");
    if (arch.equals("ppc64le") || arch.equals("ppc64")) {
      // Since java.nio.Bits.unaligned() doesn't return true on ppc (See JDK-8165231), but
      // ppc64 and ppc64le support it
      _unaligned = true;
    } else {
      try {
        Class<?> bitsClass =
          Class.forName("java.nio.Bits", false, ClassLoader.getSystemClassLoader());
        Method unalignedMethod = bitsClass.getDeclaredMethod("unaligned");
        unalignedMethod.setAccessible(true);
        _unaligned = Boolean.TRUE.equals(unalignedMethod.invoke(null));
      } catch (Throwable t) {
        // We at least know x86 and x64 support unaligned access.
        //noinspection DynamicRegexReplaceableByCompiledPattern
        _unaligned = arch.matches("^(i[3-6]86|x86(_64)?|x64|amd64|aarch64)$");
      }
    }
    unaligned = _unaligned;
  }

  /**
   * @return true when running JVM is having sun's Unsafe package available in it and underlying
   *         system having unaligned-access capability.
   */
  public static boolean unaligned() {
    return unaligned;
  }

  public static int getInt(Object object, long offset) {
    return _UNSAFE.getInt(object, offset);
  }

  public static void putInt(Object object, long offset, int value) {
    _UNSAFE.putInt(object, offset, value);
  }

  public static boolean getBoolean(Object object, long offset) {
    return _UNSAFE.getBoolean(object, offset);
  }

  public static void putBoolean(Object object, long offset, boolean value) {
    _UNSAFE.putBoolean(object, offset, value);
  }

  public static byte getByte(Object object, long offset) {
    return _UNSAFE.getByte(object, offset);
  }

  public static void putByte(Object object, long offset, byte value) {
    _UNSAFE.putByte(object, offset, value);
  }

  public static short getShort(Object object, long offset) {
    return _UNSAFE.getShort(object, offset);
  }

  public static void putShort(Object object, long offset, short value) {
    _UNSAFE.putShort(object, offset, value);
  }

  public static long getLong(Object object, long offset) {
    return _UNSAFE.getLong(object, offset);
  }

  public static void putLong(Object object, long offset, long value) {
    _UNSAFE.putLong(object, offset, value);
  }

  public static float getFloat(Object object, long offset) {
    return _UNSAFE.getFloat(object, offset);
  }

  public static void putFloat(Object object, long offset, float value) {
    _UNSAFE.putFloat(object, offset, value);
  }

  public static double getDouble(Object object, long offset) {
    return _UNSAFE.getDouble(object, offset);
  }

  public static void putDouble(Object object, long offset, double value) {
    _UNSAFE.putDouble(object, offset, value);
  }

  public static Object getObjectVolatile(Object object, long offset) {
    return _UNSAFE.getObjectVolatile(object, offset);
  }

  public static void putObjectVolatile(Object object, long offset, Object value) {
    _UNSAFE.putObjectVolatile(object, offset, value);
  }

  public static long allocateMemory(long size) {
    return _UNSAFE.allocateMemory(size);
  }

  public static void freeMemory(long address) {
    _UNSAFE.freeMemory(address);
  }
  
  public static long reallocateMemory(long address, long oldSize, long newSize) {
    long newMemory = _UNSAFE.allocateMemory(newSize);
    copyMemory(null, address, null, newMemory, oldSize);
    freeMemory(address);
    return newMemory;
  }
 
 ////////////////////////////// NVM UNSAFE FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

  public static long nvmInitializeMemory(String path, long size) {
      path = "/mnt/pmemdir/executor";
      return _NVM_UNSAFE.nvmInitialPool(path, size);
  }

  public static long nvmAllocateMemory(long size) {
      return _NVM_UNSAFE.nvmAllocateMemory(PMEM_ADDR, size);
  }
  
  public static void nvmFreeMemory(long address) {
    _NVM_UNSAFE.nvmFreeMemory(PMEM_ADDR, address);
  }

  /**
   * Uses internal JDK APIs to allocate a DirectByteBuffer while ignoring the JVM's
   * MaxDirectMemorySize limit (the default limit is too low and we do not want to require users
   * to increase it).
   */
  @SuppressWarnings("unchecked")
  public static ByteBuffer allocateDirectBuffer(int size) {
    try {
      Class<?> cls = Class.forName("java.nio.DirectByteBuffer");
      Constructor<?> constructor = cls.getDeclaredConstructor(Long.TYPE, Integer.TYPE);
      constructor.setAccessible(true);
      Field cleanerField = cls.getDeclaredField("cleaner");
      cleanerField.setAccessible(true);
      long memory = allocateMemory(size);
      ByteBuffer buffer = (ByteBuffer) constructor.newInstance(memory, size);
      Cleaner cleaner = Cleaner.create(buffer, () -> freeMemory(memory));
      cleanerField.set(buffer, cleaner);
      return buffer;
    } catch (Exception e) {
      throwException(e);
    }
    throw new IllegalStateException("unreachable");
  }

  /**
   * This function allocate a direct buffer on the persistent memory Uses internal JDK APIs to
   * allocate a DirectByteBuffer while ignoring the JVM's MaxDirectMemorySize limit (the default
   * limit is too low and we do not want to require users to increase it).
   * 
   * @param     size    The size of the direct buffer
   * @return            The allocated ByteBuffer
   *
   * @author Jack Kolokasis (08/10/18)
   */
  public static ByteBuffer allocatePmemDirectBuffer(int size) {
    try {
      Class<?> cls = Class.forName("java.nio.DirectByteBuffer");
      Constructor<?> constructor = cls.getDeclaredConstructor(Long.TYPE, Integer.TYPE); 
      constructor.setAccessible(true);
      Field cleanerField = cls.getDeclaredField("cleaner");
      cleanerField.setAccessible(true);

      /** Allocate persistent memory */
      long nvMemory = nvmAllocateMemory(size);

      /** Construct a byte buffer instance using the address on the allocated persistent memory and
       * the size
       */
      ByteBuffer buffer = (ByteBuffer) constructor.newInstance(nvMemory, size);
      /** 
       * Create a cleaner manager for the object. The cleaner manager use pmemFreeMemory function to
       * clean the object and produce the deallocation operation
       */
      Cleaner cleaner = Cleaner.create(buffer, () -> nvmFreeMemory(nvMemory));

      /** Assign the cleaner manager to the object */
      cleanerField.set(buffer, cleaner);
      
      return buffer;

    } catch (Exception e) { 
        throwException(e);
    }
    throw new IllegalStateException("unreachable");
  }

  public static void setMemory(Object object, long offset, long size, byte value) {
    _UNSAFE.setMemory(object, offset, size, value);
  }

  public static void setMemory(long address, byte value, long size) {
  }

  public static void copyMemory(
    Object src, long srcOffset, Object dst, long dstOffset, long length) {
    // Check if dstOffset is before or after srcOffset to determine if we should copy
    // forward or backwards. This is necessary in case src and dst overlap.
    if (dstOffset < srcOffset) {
      while (length > 0) {
        long size = Math.min(length, UNSAFE_COPY_THRESHOLD);
        _UNSAFE.copyMemory(src, srcOffset, dst, dstOffset, size);
        length -= size;
        srcOffset += size;
        dstOffset += size;
      }
    } else {
      srcOffset += length;
      dstOffset += length;
      while (length > 0) {
        long size = Math.min(length, UNSAFE_COPY_THRESHOLD);
        srcOffset -= size;
        dstOffset -= size;
        _UNSAFE.copyMemory(src, srcOffset, dst, dstOffset, size);
        length -= size;
      }
    }
  }

  /**
   * Raises an exception bypassing compiler checks for checked exceptions.
   */
  public static void throwException(Throwable t) {
    _UNSAFE.throwException(t);
  }

  /**
   * Limits the number of bytes to copy per {@link Unsafe#copyMemory(long, long, long)} to
   * allow safepoint polling during a large copy.
   */
  private static final long UNSAFE_COPY_THRESHOLD = 1024L * 1024L;

  static {
    sun.misc.Unsafe unsafe;
    com.nvmUnsafe.NVMUnsafe nvmUnsafe;

    try {
      Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
      unsafeField.setAccessible(true);
      unsafe = (sun.misc.Unsafe) unsafeField.get(null);
    } catch (Throwable cause) {
      unsafe = null;
    }
    _UNSAFE = unsafe;

    try {
        Field nvmUnsafeField = NVMUnsafe.class.getDeclaredField("theNVMUnsafe");
        nvmUnsafeField.setAccessible(true);
        nvmUnsafe = (com.nvmUnsafe.NVMUnsafe) nvmUnsafeField.get(null);
    } catch (Throwable cause) {
        nvmUnsafe = null;
    }

    _NVM_UNSAFE = nvmUnsafe;

    if (_UNSAFE != null) {
      BOOLEAN_ARRAY_OFFSET  = _UNSAFE.arrayBaseOffset(boolean[].class);
      BYTE_ARRAY_OFFSET     = _UNSAFE.arrayBaseOffset(byte[].class);
      SHORT_ARRAY_OFFSET    = _UNSAFE.arrayBaseOffset(short[].class);
      INT_ARRAY_OFFSET      = _UNSAFE.arrayBaseOffset(int[].class);
      LONG_ARRAY_OFFSET     = _UNSAFE.arrayBaseOffset(long[].class);
      FLOAT_ARRAY_OFFSET    = _UNSAFE.arrayBaseOffset(float[].class);
      DOUBLE_ARRAY_OFFSET   = _UNSAFE.arrayBaseOffset(double[].class);
      PMEM_ADDR             = 0;
    } 

    else if (_NVM_UNSAFE != null){
      BOOLEAN_ARRAY_OFFSET  = 0;
      BYTE_ARRAY_OFFSET     = 0;
      SHORT_ARRAY_OFFSET    = 0;
      INT_ARRAY_OFFSET      = 0;
      LONG_ARRAY_OFFSET     = 0;
      FLOAT_ARRAY_OFFSET    = 0;
      DOUBLE_ARRAY_OFFSET   = 0;
      PMEM_ADDR             = 0;

      // PMEM_BOOLEAN_ARRAY_OFFSET = 0; // _NVM_UNSAFE.arrayBaseOffset(boolean[].class);
      // PMEM_BYTE_ARRAY_OFFSET    = 0; // _NVM_UNSAFE.arrayBaseOffset(byte[].class);
      // PMEM_SHORT_ARRAY_OFFSET   = 0; // _NVM_UNSAFE.arrayBaseOffset(short[].class);
      // PMEM_INT_ARRAY_OFFSET     = 0; // _NVM_UNSAFE.arrayBaseOffset(int[].class);
      // PMEM_LONG_ARRAY_OFFSET    = 0; // _NVM_UNSAFE.arrayBaseOffset(long[].class);
      // PMEM_FLOAT_ARRAY_OFFSET   = 0; // _NVM_UNSAFE.arrayBaseOffset(float[].class);
      // PMEM_DOUBLE_ARRAY_OFFSET  = 0; // _NVM_UNSAFE.arrayBaseOffset(double[].class);
    }
    else {
      BOOLEAN_ARRAY_OFFSET      = 0;
      BYTE_ARRAY_OFFSET         = 0;
      SHORT_ARRAY_OFFSET        = 0;
      INT_ARRAY_OFFSET          = 0;
      LONG_ARRAY_OFFSET         = 0;
      FLOAT_ARRAY_OFFSET        = 0;
      DOUBLE_ARRAY_OFFSET       = 0;
      PMEM_ADDR                 = 0;
      //PMEM_PATH = "/mnt/pmemdir/executor";
    }
  }
}
