/* src/com/nvmUnsafe/NVMUnsafe.scala */
package com.teraCache;

import java.lang.instrument.Instrumentation;
import java.lang.reflect.*;
import java.security.*;
import scala.reflect.ClassTag;


public final class TeraCache {
   static {
      System.loadLibrary("TeraCache");      /* Load native library at runtime */
                                            /* libTeraCache.so (Unix)         */
   }
   
   /**
    * @desc Print hello
    *
    */
   public native void    sayHello();

   /**
    * @desc Allocate string object directly to Old generation
    *       JVM will mark this object to be transfered in TeraCache
    *
    * @param str String object
    *
    * @ret Return allocated object
    *
    */
   public native Object    test_class(String name, String sur);

   /**
    * @desc Allocate object directly to Old generation
    *       JVM will mark this object to be transfered in TeraCache
    *
    * @param cache_obj Objec to allocate
    *
    * @ret Return obj
    *
    */
   public native Object  cache(Object value, long size, ClassTag classTag);
   

}
