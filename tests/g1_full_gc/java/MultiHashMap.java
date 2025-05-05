import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;
import java.util.HashMap;

import java.lang.reflect.Field;

public class MultiHashMap {
  static long num = 0;
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



  public static void main(String[] args) {
    int num_elements = 10000;
    int sub_elem = 5000;

    HashMap<String, ArrayList<Integer>> h_map = new HashMap<String, ArrayList<Integer>>();
    _UNSAFE.h2TagAndMoveRoot(h_map, 0, 0);

    for (int i = 0; i < num_elements/2; i++)
    {
      String str = new String("Iacovos Kolokasis " + i);
      _UNSAFE.h2TagAndMoveRoot(str, 1, 0);

      ArrayList<Integer> array = new ArrayList<Integer>();
      for (int j = 0; j < sub_elem; j++) {
        Integer number = new Integer(j * 10);
        array.add(number);
      }

      h_map.put(str, array);

    }

    gc();

    h_map.entrySet().forEach(entry -> {
      num += entry.getKey().hashCode();
      ArrayList<Integer> arr = entry.getValue();

      // gc();
      for (int i = 0; i < sub_elem; i++)
      {
        num += arr.get(i).hashCode();
      }
    });
  }
}
