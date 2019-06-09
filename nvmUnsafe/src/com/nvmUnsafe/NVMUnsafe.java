/* src/com/nvmUnsafe/NVMUnsafe.scala */
package com.nvmUnsafe;

import java.lang.instrument.Instrumentation;
import java.lang.reflect.*;
import java.security.*;

public final class NVMUnsafe {
   static {
      System.loadLibrary("NVMUnsafe");      /* Load native library at runtime */
                                            /* libNVMUnsafe.so (Unix)         */
      sun.reflect.Reflection.registerMethodsToFilter(NVMUnsafe.class, "getNVMUnsafe");
   }

   private NVMUnsafe() {}

   private static final  NVMUnsafe theNVMUnsafe = new NVMUnsafe();
   
   /**
    * Provides the caller with the capability of performing nvmunsafe
    * operations.
    *
    * The returned NVMUnsafe object should be carefully guarded by the
    * caller, since it can be used to read and write data an arbitary
    * memory addresses. It must never be passed to untrusted code.
    *
    * Most methods in this class are very low-level, and correspond to
    * a small number of hardware instructions (on typical machines).
    * Compilers are encouraged to optimize these methods accordingly.
    *
    * Here is a suggested idiom for using unsafe operations:
    *
    * class MyTrustedClass {
    *   private static final NVMUnsafe nvmunsafe = NVMUnsafe.getUnsafe();
    *   ...
    *   private long myCountAddress = ...;
    *   public int getCount() { return nvmunsafe.getByte(myCountAddress); }
    * }
    *
    * (It may assist compilers to make the local variable be final)
    */
   public static NVMUnsafe getNVMUnsafe() {
       Class cc = sun.reflect.Reflection.getCallerClass(2);

       if (cc.getClassLoader() != null)
           throw new SecurityException("NVMUnsafe");

       return theNVMUnsafe;
   }

   /**
    * @desc Initialize non volatile memory pool
    *
    * @param path      NVM file path
    * @param pool_size Total memory pool size
    *
    * @ret Start address of the nvm allocated pool size
    *
    */
   public native void    nvmInitialPool(String path, long pool_size);


   /**
    * @desc Allocate Memory for an Object using size
    *
    * @param pmp       Start Address persistent memory pool
    * @param size      Object size
    *
    * @ret Allocation address
    *
    */
   public native long    nvmAllocateMemory(long size);

   /**
    * @desc Get an Integer saved on Persistent Memory
    *
    * @param pmp       Start Address persistent memory pool
    * @param offset    Allocation address
    *
    * @ret the integer value saved on the persistent memory
    */
   public native int     getInt(long offset);

   /**
    * @desc save an integer on persistent memory
    *
    * @param value     integer object
    * @param pmp       start address persistent memory pool
    * @param offset    allocation address
    *
    */
   public native void    putInt(int number, long offset);
  
   /**
    * @desc Get boolean saved in persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    * @ret return saved boolean
    */
   public native boolean getBoolean(long offset);

   /**
    * @desc Save a Boolean on Persistent Memory
    *
    * @param bool      Boolean object
    * @param pmp       Start Address persistent memory pool
    * @param offset    Allocation address
    */
   public native void    putBoolean(boolean bool, long offset);
  
   /**
    * @desc Get byte saved in persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native byte    getByte(long offset);

   /**
    * @desc Put byte in persistent memory pool
    *
    * @param obj       Object to save in persistent memory
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native void    putByte(byte obj, long offset);
   
   /**
    * @desc Get short saved in persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native short   getShort(long offset);

   /**
    * @desc Save a short on Persistent Memory
    *
    * @param value     Short object
    * @param pmp       Start Address persistent memory pool
    * @param offset    Allocation address
    *
    */
   public native void    putShort(short obj, long offset);

   /**
    * @desc Get a long from persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    * @ret Return the long object
    *
    */
   public native long    getLong(long offset);

   /**
    * @desc Put a long in persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native void    putLong(long number, long offset);

   /**
    * @desc Get a float from persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native float   getFloat(long offset);

   /**
    * @desc Put a float in persistent memory pool
    *
    * @param value     Value for allocation
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native void    putFloat(float obj, long offset);


   /**
    * @desc Get a double from persistent memory pool
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native double  getDouble(long offset);

   /**
    * @desc Put a double in persistent memory pool
    *
    * @param value     Value for allocation
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native void    putDouble(double number, long offset);

   /**
    * @desc Free memory
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   public native void    nvmFreeMemory(long offset);
   
   /**
    * @desc Get translated pmem address
    *
    * @param pmp       Start address of the memory pool
    * @param offset    Allocation address
    *
    */
   // public native long getPmemAddress(long pmp, long offset);
   
   /**
    * @desc Release the memory pool vmp. If the memory pool was
    * created using vmem_create(), deleting it allows the space to be
    * reclaimed.
    *
    */
    public native void nvmDelete();
    
}
