import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;

import java.lang.reflect.Field;

public class HashMap {
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

  public static void main(String[] args) {

    int num_elements = 1000000;
    // int num_elements = 2000000;
    // int num_elements = Integer.parseInt(args[0]);

    long check = 0;

    System.out.println(Runtime.getRuntime().maxMemory());

    ConcurrentHashMap<String, String> h_map = new ConcurrentHashMap<String, String>();
    _UNSAFE.h2TagAndMoveRoot(h_map, 0, 0);

    for (int i = 0; i < num_elements/2; i++){
      h_map.put("Bangalore" + i, "22");
    }

    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()){
      check += Long.parseLong(e.getValue());
    }

    GC.move_to_old();
    GC.gc();

    for (int i = 0; i < num_elements/2; i++){
      h_map.put("Ammochostos " + i, "20404808");
    }

    GC.move_to_old();
    GC.gc();

    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()){
      check += Long.parseLong(e.getValue());
    }

    GC.gc();

    for (int i = num_elements/2; i < num_elements; i++){
      h_map.put("Apostolos Andreas" + i, "22");
    }

    GC.move_to_old();
    GC.gc(); 

    for (int i = num_elements/2; i < num_elements; i++){
      h_map.put("Moires" + i, "22"); 
    }

    GC.gc();

    check = 0;
    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet())
      check += Long.parseLong(e.getValue());

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Paphos" + i, "22");

    GC.move_to_old();
    GC.gc();

    check = 0;
    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet())
      check += Long.parseLong(e.getValue());

    GC.gc();

    for (int i = num_elements/2; i < num_elements; i++){
      h_map.put("Karpasia" + i, "22");
    }

    h_map.put("Mia milia", "22");
    GC.move_to_old();	

    GC.gc(); 

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Kerineia" + i, "22");

    check = 0;
    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet())
      check += Long.parseLong(e.getValue());

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Heraklion" + i, "22");

    GC.gc(); 	

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Troodos" + i, "22");

    check = 0;
    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet())
      check += Long.parseLong(e.getValue());

    GC.move_to_old();
    GC.gc(); 

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Achna" + i, "22");

    check = 0;
    for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet())
      check += Long.parseLong(e.getValue());

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Maria" + i, "22");

    h_map.clear();

    GC.move_to_old();
    GC.gc();

    for (int i = num_elements/2; i < num_elements; i++)
      h_map.put("Maria" + i, "22");

    h_map = null;

    GC.gc();
  }
}
