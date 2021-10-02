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

public class Groupping 
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
		// declares an Array of integers. 
		// allocating memory for 5 integers. 
		//
		int num_elements = 20000;
		int num_elements_2 = 10000000;

		mem_info("Memory Before");

		// Create the array list
		LinkedList<Integer> arl = new LinkedList<Integer>();
		// Add data to the list
		
        /*
		gc();
        
		gc();
        */
		long x = 0;
		// Traverse all the data of the list
		for (int i = 0; i < num_elements; i++)
		{
            Integer int1 = new @Cache Integer(5);
            arl.add(int1);
		}
        gc();
        gc();
        for (int i = 0; i < num_elements; i++)
		{
            arl.remove();
		}
        gc();
        gc();
		LinkedList<String> arl2 = new LinkedList<String>();
        
        for (int i = 0; i < num_elements_2; i++)
		{
            String int1 = new @Cache String("ttt");
            arl2.add(int1);
		}
        gc();
        gc();
       /* 
		x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);


		gc();

		x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);

		gc();

		x = 0;
		System.gc();
		for (int i = 0; i < num_elements; i++)
		{
			x += arl.get(i).hashCode();
		}

		System.out.println("Hashcode Element = " + x);

		gc();
	*/	
		mem_info("Memory After");
	} 
} 

