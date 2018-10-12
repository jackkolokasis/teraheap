/* src/com/nvmUnsafe/NVMUnsafe.scala */
package com.nvmUnsafe;

import java.lang.instrument.Instrumentation;

public class NVMUnsafe {
   static {
      System.loadLibrary("NVMUnsafe");      /* Load native library at runtime */
                                            /* libNVMUnsafe.so (Unix)         */
   }
 
   public native long   nvmInitialPool(String path, long pool_size);
   public native long   nvmAllocateMemory(long pmp, long size);

   public native int    getInt(long pmp, long offset);
   public native void   putInt(int number, long pmp, long offset);
  
   public native boolean getBoolean(long pmp, long offset);
   public native void   putBoolean(boolean bool, long pmp, long offset);
  
   public native byte   getByte(long pmp, long offset);
   public native void   putByte(byte obj, long pmp, long offset);
   
   public native short  getShort(long pmp, long offset);
   public native void   putShort(short obj, long pmp, long offset);

   public native long   getLong(long pmp, long offset);
   public native void   putLong(long number, long pmp, long offset);
   
   public native float  getFloat(long pmp, long offset);
   public native void   putFloat(float obj, long pmp, long offset);
   
   public native double getDouble(long pmp, long offset);
   public native void   putDouble(long number, long pmp, long offset);

   public native void  nvmFreeMemory(long pmp, long offset);
    
}
