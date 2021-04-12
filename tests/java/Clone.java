import java.io.*; 
import java.lang.*; 
import java.util.Scanner;
import java.util.ArrayList;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
 
public class Clone 
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
    @SuppressWarnings("unchecked")
    public static void main(String[] args) 
    {
		int num_elements = 1000000;

		System.out.println(Runtime.getRuntime().maxMemory());

        ArrayList<String> arrayListObject = new @Cache ArrayList<>(); 

		gc();

		for (int i = 0; i < num_elements/2; i++)
		{
			arrayListObject.add("Jack Kolokasis " + i);
		}

		gc();
		
		for (int i = 0; i < num_elements/2; i++)
		{
			arrayListObject.add("Jack Kolokasis " + i);
		}
         
        System.out.println(arrayListObject);   
		
		for (int i = 0; i < num_elements/2; i++)
		{
			arrayListObject.add("Jack Kolokasis " + i);
		}

		gc();
         
        ArrayList<String> arrayListClone =  (ArrayList<String>) arrayListObject.clone();

		gc();
         
        System.out.println(arrayListClone);   
    }
}
