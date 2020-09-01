// Test migration of an object and all the objects that points out
//
import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.LinkedList;

public class List_Large {

	public static void main(String[] args) {
		int num_elements = 10000;

		System.out.println("=========================================");
		System.out.println("Memory Before\n");
		System.out.println("=========================================");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
		System.out.println("=========================================");


		System.out.println("=========================================");
		System.out.println("Create List");
		System.out.println("=========================================");
		LinkedList<Integer> linkedList = new @Cache LinkedList<Integer>(); 
		System.out.println("=========================================");


		System.out.println("=========================================");
		System.out.println("Create String");
		System.out.println("=========================================");
		String str = new @Cache String("lala");


		System.out.println("=========================================");
		System.out.println("Add Elements to the  List");
		System.out.println("=========================================");

		for (int i = 0; i < num_elements; i++)
		{
			linkedList.add(i);
		}


		System.out.println("=========================================");
		System.out.println("Memory After\n");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
		System.out.println("=========================================");

		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		int x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			x += linkedList.get(i).hashCode();
		}

		System.out.println("=========================================");
		System.out.println("Access Cached Data");
		System.out.println("First Element = " + linkedList.getFirst());
		System.out.println("Str = " + str);
		System.out.println("Hashcode Element = " + x);
		
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");


		System.out.println("=========================================");
		System.out.println("Memory After\n");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
		System.out.println("=========================================");


		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");


		System.out.println("=========================================");
		System.out.println("Memory After\n");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
		System.out.println("=========================================");
	}
}
