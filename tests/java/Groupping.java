// Java program to illustrate creating an array 
// of integers, puts some values in the array, 
// and prints each value to standard output. 

import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.LinkedList;
import java.util.ArrayList;
import java.lang.reflect.Field;

public class Groupping 
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
	
	public static void calcHashCode(LinkedList<String> list, int num_elements) {
		long sum = 0;

		for (int i = 0; i < num_elements; i++)
			sum += list.get(i).hashCode();

		System.out.println("Hashcode Element = " + sum);
	}

	public static void main (String[] args) 
	{		 
		int num_elements = 20000;
		int num_elements_2 = 10000000;

		mem_info("Memory Before");

		LinkedList<Integer> list = new LinkedList<Integer>();
		_UNSAFE.h2TagAndMoveRoot(list, 0, 0);

		gc();
        
		long x = 0;
		// Traverse all the data of the list
		for (int i = 0; i < num_elements; i++)
			list.add(new Integer(i));

        gc();

        for (int i = 0; i < num_elements; i++)
            list.remove();

        gc();
		LinkedList<String> list2 = new LinkedList<String>();
		_UNSAFE.h2TagAndMoveRoot(list2, 1, 0);
        
        for (int i = 0; i < num_elements_2; i++)
            list2.add(new String("Hello World " + i));

        gc();
		calcHashCode(list2, num_elements);

		gc();
		calcHashCode(list2, num_elements);

		gc();
		calcHashCode(list2, num_elements);

		mem_info("Memory After");
	} 
} 

