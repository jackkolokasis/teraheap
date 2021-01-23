import java.lang.ref.Reference;
import java.lang.ref.SoftReference;
import java.lang.ref.WeakReference;

public class Test_Reference {
    public static class Bean {

        String name;
        Integer va;
        public Bean(String name, Integer value) {
            this.name = name;
            this.va = value;
        }

        @Override
        public String toString() {
            // TODO Auto-generated method stub
            return name +": " + va;
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
        //Strong Reference

        Bean bean = new Bean("songli", 67);
         Bean[] referent = new Bean[20000]; 
         for (int i=0;i<referent.length;i++){ 
             referent[i] = new Bean("mybean:" + i,100);// Throw Exception 
         }
         System.out.println(referent[1000].toString());

        //Soft reference
         Reference<Bean>[] soft_referent = new SoftReference[20000]; 
         for (int i=0;i<soft_referent.length;i++){ 
             soft_referent[i] = new SoftReference<Bean>(new Bean("mybean:" + i,100)); 
         } 
         System.out.println("finish");
         System.out.println(soft_referent[499].get() + " " + 
                 soft_referent[10000].get() + " " + 
                 soft_referent[19999].get());// "null"
////         
        WeakReference<Bean>[] weakReference = new @Cache WeakReference[2000];
        System.gc();
        System.gc();

        System.out.println(weakReference[1000]); //
        System.gc();
    }
}
