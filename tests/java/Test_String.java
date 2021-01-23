import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.util.LinkedList;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;

// Simple Object creation and transfer to TeraCache

class Test_String
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

	public static void main(String args[]) 
	{ 
		mem_info("Mem Info:");
		gc();

		String str = new @Cache String("Hello World");

		gc();

		System.out.println("String = " + str);
		mem_info("Mem Info:");
	} 
} 


