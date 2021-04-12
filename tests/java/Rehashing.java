import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.HashMap;

public class Rehashing {

	public static void mem_info(String str) {
		System.out.println("=========================================");
		System.out.println(str + "\n");
		System.out.println("=========================================");
		for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
			System.out.println(memoryPoolMXBean.getName());
			System.out.println(memoryPoolMXBean.getUsage().getUsed());
		}
	}

	public static void gc() {
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");
	}

  public static void main(String[] args) {
	  int num_elements = 1000000;

    // Create a HashMap object called people
    HashMap<String, Integer> people = new @Cache HashMap<String, Integer>(2, 0.75f);

    // Add keys and values (Name, Age)
	for (int i = 0; i < num_elements/2; i++) {
		people.put("Jack " + i, 100);
	}

    people.put("John", 32);
    people.put("Steve", 30);
    people.put("Angie", 33);

	gc();

    for (String i : people.keySet()) {
      System.out.println("key: " + i + " value: " + people.get(i));
    }

	gc();
	
	for (int i = 0; i < num_elements/2; i++) {
		people.put("Rafail " + i, 100);
	}
	
	gc();

	gc();
	
	for (int i = 0; i < num_elements/2; i++) {
		people.put("Pavlos " + i, 100);
	}
	
	gc();
    
	for (String i : people.keySet()) {
      System.out.println("key: " + i + " value: " + people.get(i));
    }
	
	gc();
	
	for (int i = 0; i < num_elements/2; i++) {
		people.put("Iacovos " + i , 100);
	}
	
	gc();
	
	for (String i : people.keySet()) {
      System.out.println("key: " + i + " value: " + people.get(i));
    }


  }
}
