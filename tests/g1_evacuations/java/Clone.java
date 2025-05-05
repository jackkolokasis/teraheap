import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.util.ArrayList;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.lang.reflect.Field;

public class Clone 
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

  @SuppressWarnings("unchecked")
  public static void main(String[] args) 
  {
    int num_elements = 4000000;

    System.out.println(Runtime.getRuntime().maxMemory());

    ArrayList<String> arrayListObject = new ArrayList<>(); 
    _UNSAFE.h2TagAndMoveRoot(arrayListObject, 0, 0);


    for (int i = 0; i < num_elements/2; i++)
    {
      arrayListObject.add("Jack Kolokasis " + i);
    }

    GC.move_to_old();
    GC.gc();

    for (int i = 0; i < num_elements/2; i++)
    {
      arrayListObject.add("Jack Kolokasis " + i);
    }

    GC.move_to_old();
    GC.gc();

    System.out.println(arrayListObject);   

    for (int i = 0; i < num_elements/4; i++)
    {
      arrayListObject.add("Nicos Kolokasis " + i);
    }

    GC.move_to_old();
    GC.gc();

    ArrayList<String> arrayListClone =  (ArrayList<String>) arrayListObject.clone();

    GC.move_to_old();
    GC.gc();

    System.out.println(arrayListClone);   

    for (int i = 0; i < num_elements/4; i++)
    {
      arrayListObject.add("Jack Kolokasis " + i + "00");
    }

    GC.move_to_old();
    GC.gc();

  }
}
