import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.util.LinkedList;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;

// Java program to demonstrate lambda expressions 
// to implement a user defined functional interface. 

// A sample functional interface (An interface with 
// single abstract method 
interface FuncInterface 
{ 
	// An abstract function 
	void abstractFun(int x); 

	// A non-abstract (or default) function 
	default void normalFun() 
	{ 
	System.out.println("Hello"); 
	} 
} 

class Extend_Lambda
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
		// Lambda expression to implement above 
		// functional interface. This interface 
		// by default implements abstractFun() 
		FuncInterface fobj = (int x)->System.out.println(2*x); 
		int num_elements = 50000;
		

		mem_info("Memory Information");
		gc();
		mem_info("Memory Information");
	
		fobj.abstractFun(5); 

		LinkedList<Integer> linkedList = new @Cache LinkedList<Integer>(); 

		mem_info("Memory Information");
		gc();
		mem_info("Memory Information");

		for (int i = 0; i < num_elements; i++)
		{
			linkedList.add(new Integer(i));
		}

		fobj.abstractFun(1000); 
		
		mem_info("Memory Information");
		gc();
		mem_info("Memory Information");

		fobj.abstractFun(2000); 

		
		LinkedList<Integer> linkedList2 = new LinkedList<Integer>(); 
		for (int i = 0; i < num_elements; i++)
		{
			linkedList2.add(new Integer(i));
		}

		String str = new @Cache String("Jack Kolokasis");
		
		fobj.abstractFun(4000); 
	
		mem_info("Memory Information");
		gc();
		mem_info("Memory Information");
		
		int x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			x += linkedList.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);
		
		x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			System.out.println(linkedList2.get(i));
		}

		System.out.println("Hashcode Element = " + x);
		fobj.abstractFun(4000); 
		
		mem_info("Memory Information");
		gc();
		mem_info("Memory Information");
		
		x = 0;
		for (int i = 0; i < num_elements; i++)
		{
			x += linkedList2.get(i).hashCode();
		}
		System.out.println("Hashcode Element = " + x);
		System.out.println("String = " + str);
		
		mem_info("Memory Information");
		gc();
		mem_info("Memory Information");
	} 
} 
