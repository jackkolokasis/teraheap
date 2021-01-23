import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;

public class HashMap {

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

	public static void main(String[] args) {
		int num_elements = 10000000;

		System.out.println(Runtime.getRuntime().maxMemory());
		
		// Creating ConcurrentHashMap
		ConcurrentHashMap<String, String> h_map = new @Cache ConcurrentHashMap<String, String>();

		// Storing elements
		for (int i = 0; i < num_elements/2; i++)
		{
			h_map.put("Bangalore" + i, "22");
		}

		gc();
		
		h_map.put("Dali", "22");
		gc();
		h_map.put("Nicosia", "22");

		for (int i = 0; i < num_elements/2; i++)
		{
			h_map.put("Ammochostos " + i, "20404808");
		}
		
		gc();

		for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()) {
			System.out.println(e.getKey() + " = " + e.getValue());
		}
		
		gc();
		
		for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()) {
			System.out.println(e.getKey() + " = " + e.getValue());
		}

		h_map.put("Ammochostos", "5608");
		
		for (int i = num_elements/2; i < num_elements; i++)
		{
			h_map.put("Paphos" + i, "22");
		}
		
		gc();

		for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()) {
			System.out.println(e.getKey() + " = " + e.getValue());
		}

		gc();

		h_map.put("Mia milia", "22");
		
		gc();
		
		for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()) {
			System.out.println(e.getKey() + " = " + e.getValue());
		}
		
		for (int i = num_elements/2; i < num_elements; i++)
		{
			h_map.put("Heraklion" + i, "22");
		}
		gc();
		for (ConcurrentHashMap.Entry<String, String> e : h_map.entrySet()) {
			System.out.println(e.getKey() + " = " + e.getValue());
		}
	}
}
