import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.HashMap;

import java.lang.reflect.Field;

public class Rehashing {
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

	public static void gc() {
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");
	}

	public static void main(String[] args) {
		int num_elements = 1000000;
		long sum;

		HashMap<String, Integer> people = new HashMap<String, Integer>(2, 0.75f);
		_UNSAFE.h2TagAndMoveRoot(people, 0, 0);

		for (int i = 0; i < num_elements/2; i++)
			people.put("Jack " + i, 100);

		people.put("John", 32);
		people.put("Steve", 30);
		people.put("Angie", 33);

		gc();

		sum = 0;
		for (String i : people.keySet())
			sum += people.get(i);

		gc();

		for (int i = 0; i < num_elements/2; i++)
			people.put("Rafail " + i, 100);

		gc();

		for (int i = 0; i < num_elements/2; i++) {
			people.put("Pavlos " + i, 100);
		}

		gc();

		for (String i : people.keySet())
			sum += people.get(i);

		gc();

		for (int i = 0; i < num_elements/2; i++)
			people.put("Iacovos " + i , 100);

		gc();

		sum = 0;
		for (String i : people.keySet())
			sum += people.get(i);
	}
}
