// Test migration of an object and all the objects that points out
//
import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.util.LinkedList;

public class List_Small {

    public static void main(String[] args) {

        System.out.println("=========================================");
        System.out.println("Memory Before\n");
        for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
            System.out.println(memoryPoolMXBean.getName());
            System.out.println(memoryPoolMXBean.getUsage().getUsed());
        }
        System.out.println("=========================================");

        System.out.println("=========================================");
        System.out.println("Create List");
        System.out.println("=========================================");
        LinkedList<Integer> linkedList = new @Cache LinkedList<Integer>(); 

        System.out.println("=========================================");
        System.out.println("Add Elements to the  List");
        System.out.println("=========================================");
        for (int i = 0; i < 100; i++)
          linkedList.add(i*10);

        System.out.println("=========================================");
        System.out.println("Call GC");
        System.gc();
        System.out.println("=========================================");
        
        System.out.println("=========================================");
        System.out.println("First Element = " + linkedList.getFirst());
        System.out.println("Last Element = " + linkedList.getLast());
        System.out.println("=========================================");
        System.gc();

        System.out.println("=========================================");
        System.out.println("Memory After\n");
        for(MemoryPoolMXBean memoryPoolMXBean: ManagementFactory.getMemoryPoolMXBeans()){
            System.out.println(memoryPoolMXBean.getName());
            System.out.println(memoryPoolMXBean.getUsage().getUsed());
        }
        System.out.println("=========================================");
        
        System.out.println("=========================================");
        System.out.println("First Element = " + linkedList.getFirst());
        System.out.println("Last Element = " + linkedList.getLast());
        System.out.println("=========================================");
    }
}
