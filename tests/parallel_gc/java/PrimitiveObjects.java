import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;

import java.lang.reflect.Field;

public class PrimitiveObjects {
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
	public static void mem_info(String str) {
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

  public static void main(String args[]){
    Person p1 = new Person();
    Person p2 = new Person();
    Person p3 = new Person();
    Person p4 = new Person();
    gc();
  }
}

class Person {
  int x;
  int y;

  Person() {
    this.x = 10;
    this.y = 20;
  }

  public void set_x(int num, int num2) {
    this.x = num;
    this.y = num2;
  }

  public int getX() {
    return this.x;
  }
  
  public int getY() {
    return this.y;
  }
}
