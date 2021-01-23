import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.LinkedList;
import java.util.ArrayList;

public class Simple_Array
{ 
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

	public static void main (String[] args) 
	{		 
		int num_elements = 2000000;

		mem_info("Memory Before");

		// Declares an Array of integers. 
		// Allocating memory for <num_elements> integers. 
		int[] age = new @Cache int[2000000];
		for (int i = 0; i < num_elements; i++)
		{
			age[i] = i;
		}

		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(age[i]);
		}

		gc();

		// Traverse the data of the array
		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(age[i]);
		}

		gc();

		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(age[i]);
		}
		
		num_elements = 20000;
		int[] array_2 = new @Cache int[20000];
		for (int i = 0; i < num_elements; i++)
		{
			array_2[i] = i;
		}
		
		gc();

		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(array_2[i]);
		}

		mem_info("Memory After");
	}
}

