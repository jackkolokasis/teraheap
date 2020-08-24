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

public class Array_List 
{ 
	public static void main (String[] args) 
	{		 
		// declares an Array of integers. 
		// allocating memory for 5 integers. 
		//
		int num_elements = 10000;

		System.out.println("=========================================");
		System.out.println("Memory Before\n");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
		System.out.println("=========================================");

		// Create the array list
		ArrayList<String> arl = new @Cache ArrayList<String>();

		// Add data to the list
		for (int i = 0; i < num_elements; i++)
		{
			arl.add("Hello World!!");
		}

		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		// Add object data to the list
		for (int i = 0; i < num_elements; i++)
		{
			String str = new String("Hello World");
			arl.add(str);
		}

		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		long x = 0;
		// Traverse all the data of the list
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);


		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		for (int i = 0; i < num_elements; i++)
		{
			String str = new String("Hello World");
			arl.add(str);
		}


		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);


		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		x = 0;
		System.gc();
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);

		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		x = 0;
		System.gc();
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}

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


	} 
} 

