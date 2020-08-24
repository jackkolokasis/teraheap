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
        int num_elements = 10000000;

        System.out.println("=========================================");
        System.out.println("Memory Before\n");
        for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
            System.out.println(memoryPoolMXBean.getName());
            System.out.println(memoryPoolMXBean.getUsage().getUsed());
        }
        System.out.println("=========================================");

        System.out.println("=========================================");
        System.out.println("Create List 1");
        System.out.println("=========================================");
        LinkedList<Integer> linkedList = new @Cache LinkedList<Integer>(); 

        System.out.println("=========================================");
        System.out.println("Add Elements to the  List1");
        System.out.println("=========================================");
        for (int i = 0; i < num_elements; i++)
          linkedList.add(i);
        
        System.out.println("=========================================");
        System.out.println("Call GC");
        System.gc();
        System.out.println("=========================================");

        System.out.println("=========================================");
        System.out.println("Create List2");
        System.out.println("=========================================");
        LinkedList<Integer> linkedList2 = new @Cache LinkedList<Integer>(); 

        System.out.println("=========================================");
        System.out.println("Add Elements to the  List2");
        System.out.println("=========================================");
        for (int i = 0; i < num_elements; i++)
          linkedList2.add(i);
        
        System.out.println("=========================================");
        System.out.println("Call GC");
        System.gc();
        System.out.println("=========================================");
        
        System.out.println("=========================================");
        System.out.println("Create List3");
        System.out.println("=========================================");
        LinkedList<Integer> linkedList3 = new @Cache LinkedList<Integer>(); 

        System.out.println("=========================================");
        System.out.println("Add Elements to the  List3");
        System.out.println("=========================================");
        for (int i = 0; i < num_elements; i++)
          linkedList3.add(i);

        System.out.println("=========================================");
        System.out.println("Call GC");
        System.gc();
        System.out.println("=========================================");
        
        System.out.println("=========================================");
        System.out.println("First Element = " + linkedList.getFirst());
        System.out.println("Last Element = " + linkedList.getLast());
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
