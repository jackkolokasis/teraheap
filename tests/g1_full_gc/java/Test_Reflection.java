import java.lang.reflect.Method; 
import java.lang.reflect.Field; 
import java.lang.reflect.Constructor; 
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;

import java.lang.reflect.Field;

// class whose object is to be created 
class Test 
{ 
  // creating a private field 
  private String s; 

  // creating a public constructor 
  public Test() { s = "GeeksforGeeks"; } 

  // Creating a public method with no arguments 
  public void method1() { 
    System.out.println("The string is " + s); 
  } 

  // Creating a public method with int as argument 
  public void method2(int n) { 
    System.out.println("The number is " + n); 
  } 

  // creating a private method 
  private void method3() { 
    System.out.println("Private method invoked"); 
  } 
} 

class Test_Reflection
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



  public static void main(String args[]) throws Exception 
  { 
    // Creating object whose property is to be checked 
    Test obj = new Test(); 
    _UNSAFE.h2TagAndMoveRoot(obj, 0, 0);

    // Creating class object from the object using 
    // getclass method 
    Class cls = obj.getClass(); 
    System.out.println("The name of class is " + cls.getName()); 

    // Getting the constructor of the class through the 
    // object of the class 
    Constructor constructor = cls.getConstructor(); 
    System.out.println("The name of constructor is " + constructor.getName()); 

    System.out.println("The public methods of class are : "); 

    // Getting methods of the class through the object 
    // of the class by using getMethods 
    Method[] methods = cls.getMethods(); 

    // Printing method names 
    for (Method method:methods) 
      System.out.println(method.getName()); 

    // creates object of desired method by providing the 
    // method name and parameter class as arguments to 
    // the getDeclaredMethod 
    Method methodcall1 = cls.getDeclaredMethod("method2", int.class); 

    // invokes the method at runtime 
    methodcall1.invoke(obj, 19); 

    gc();

    // creates object of the desired field by providing 
    // the name of field as argument to the 
    // getDeclaredField method 
    Field field = cls.getDeclaredField("s"); 

    // allows the object to access the field irrespective 
    // of the access specifier used with the field 
    field.setAccessible(true); 

    gc();

    // takes object and the new value to be assigned 
    // to the field as arguments 

    field.set(obj, "JAVA"); 

    gc();

    // Creates object of desired method by providing the 
    // method name as argument to the getDeclaredMethod 
    Method methodcall2 = cls.getDeclaredMethod("method1"); 

    // invokes the method at runtime 
    methodcall2.invoke(obj); 

    field.set(obj, "Jack Kolokasis 1991"); 

    gc();
    gc();

    // invokes the method at runtime 
    methodcall2.invoke(obj); 

    gc();

    mem_info("Memory End");
  } 
} 
