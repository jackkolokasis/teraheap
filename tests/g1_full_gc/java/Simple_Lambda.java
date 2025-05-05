// Java program to demonstrate lambda expressions 
// to implement a user defined functional interface. 

// A sample functional interface (An interface with 
// single abstract method 

import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.lang.reflect.Field;

interface FuncInterface 
{ 
  // An abstract function 
  void abstractFun(int x); 

  // A non-abstract (or default) function 
  default void normalFun() 
  { 
    System.out.println("Hello"); 
  } 
} 

class Simple_Lambda 
{ 
  private static final sun.misc.Unsafe _UNSAFE;

  static {
    try {
      Field unsafeField = sun.misc.Unsafe.class.getDeclaredField("theUnsafe");
      unsafeField.setAccessible(true);
      _UNSAFE = (sun.misc.Unsafe) unsafeField.get(null);
    } catch (Exception e) {
      throw new RuntimeException("SimplePartition: Failed to " + "get unsafe", e);
    }
  }

  public static void mem_info(String str)
  {
    System.out.println("=========================================");
    System.out.println(str + "\n");
    System.out.println("=========================================");
    for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
      System.out.println(memoryPoolMXBean.getName());
      System.out.println(memoryPoolMXBean.getUsage().getUsed());
    }
  }

  public static void gc()
  {
    System.out.println("=========================================");
    System.out.println("Call GC");
    System.gc();
    System.out.println("=========================================");
  }

  public static void main(String args[]) 
  { 
    // lambda expression to implement above 
    // functional interface. This interface 
    // by default implements abstractFun() 
    FuncInterface fobj = (int x)->System.out.println(2*x); 
    _UNSAFE.h2TagAndMoveRoot(fobj, 0, 0);

    mem_info("Memory Before");

    gc();

    // This calls above lambda expression and prints 10. 
    fobj.abstractFun(5); 
    fobj.abstractFun(1000); 

    mem_info("Memory After");

    gc();

    fobj.abstractFun(20000); 

    gc();

    mem_info("Memory After");
  } 
} 
