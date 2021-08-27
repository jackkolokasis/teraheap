import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;

public class Array {
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

	public static void main(String args[]) throws Exception {
	  int num_elements = 2000000;

	  int[] array1 = new @Cache int[2000000];
	  int[] array2 = new @Cache int[2000000];

	  gc();
	      
	  for (int i = 0; i < num_elements; i++)
	  {
	      array1[i] = i*2;
	  }
	  
	  gc();
	  
	  for (int i = 0; i < num_elements; i++)
	  {
	      array1[i] = i;
	  }
	  
	  gc();
	  
	  for (int i = 0; i < num_elements; i++)
	  {
	      array1[i] = i*2;
	  }
	  
	  for (int i = 0; i < num_elements; i++)
	  {
	      array2[i] = i*4;
	  }
	  
	  gc();
	  
	  for (int i = 0; i < num_elements; i++)
	  {
	      array2[i] = 333;
	  }
	  
	  gc();
	  
	  for (int i = 0; i < num_elements; i++)
	  {
	      array1[i] = i*2;
	  }
  }

}
