import java.util.*;
import java.lang.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;
import java.lang.reflect.Field;

class ClassInstance {
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

  public static void main(String[] args) {
    Laptop[] arrayLaptop = new Laptop[1000];

    for (int i = 0; i < 1000; i++) {
      arrayLaptop[i] = new Laptop();
    }

    GC.move_to_old();

    GC.gc();

    _UNSAFE.h2TagAndMoveRoot(arrayLaptop[0], 0, 0);

    Object obj = arrayLaptop[0].getClass();
    _UNSAFE.h2TagAndMoveRoot(obj, 0, 0);

    GC.gc();
    arrayLaptop[0].add_new_element();
    arrayLaptop[40].add_new_element();
    arrayLaptop[41].add_new_element();

    GC.move_to_old();

    GC.gc();

    _UNSAFE.h2TagAndMoveRoot(arrayLaptop[10], 1, 0);

    GC.gc();

    arrayLaptop[10].add_new_element();
    arrayLaptop[42].add_new_element();
    arrayLaptop[43].add_new_element();
    arrayLaptop[44].add_new_element();

    GC.move_to_old();

    GC.gc();

    _UNSAFE.h2TagAndMoveRoot(arrayLaptop[20], 2, 0);

    for (int i=500; i < 1000; i++)
      arrayLaptop[i].add_new_element();

    for (int i=50; i < 1000; i++)
      arrayLaptop[i].add_new_element();

    GC.move_to_old();

    GC.gc();

    arrayLaptop[400].add_new_element();

    GC.move_to_old();

    GC.gc();
  }
}

class Laptop {
  private static ArrayList<Integer> ar_list;
  private ConcurrentHashMap<String, String> h_map;

  Laptop() {
    this.h_map = new ConcurrentHashMap<String, String>();

    for (int i = 0; i < 10000; i++)
    {
      h_map.put("Bangalore" + i, "22");
    }

    this.ar_list = new ArrayList<Integer>();
    for (int i = 0; i < 10000; i++)
    {
      ar_list.add(i);
    }
  }

  void laptop_method() {
    System.out.println("99% Battery available.");
  }

  void add_new_element() {
    this.ar_list.add(1000);
    this.ar_list.add(2000);
    this.ar_list.add(3000);
    this.ar_list.add(4000);
  }
}

