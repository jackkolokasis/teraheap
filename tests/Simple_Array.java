import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.LinkedList;
import java.util.ArrayList;

public class Simple_Array
{ 
	public static void main (String[] args) 
	{		 
		int num_elements = 20000;

		System.out.println("=========================================");
		System.out.println("Memory Before\n");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
		System.out.println("=========================================");

		// Declares an Array of integers. 
		// Allocating memory for <num_elements> integers. 
		int[] age = new @Cache int[20000];
		for (int i = 0; i < num_elements; i++)
		{
			age[i] = i;
		}

		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(age[i]);
		}

		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");


		// Traverse the data of the array
		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(age[i]);
		}

		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(age[i]);
		}
		
		num_elements = 20;
		int[] array_2 = new @Cache int[20];
		for (int i = 0; i < num_elements; i++)
		{
			array_2[i] = i;
		}
		
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(array_2[i]);
		}
	}
}

