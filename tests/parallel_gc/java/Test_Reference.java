import java.lang.ref.Reference;
import java.lang.ref.SoftReference;
import java.lang.ref.WeakReference;

import java.lang.reflect.Field;

public class Test_Reference {
	private static final sun.misc.Unsafe _UNSAFE;

	static {
		try {
			Field unsafeField = sun.misc.Unsafe.class.getDeclaredField("theUnsafe");
			unsafeField.setAccessible(true);
			_UNSAFE = (sun.misc.Unsafe) unsafeField.get(null);
		} catch (Exception e) {
			throw new RuntimeException("SimplePartition: Failed to " + "get unsafe", e);
		}
	}

    public static class Bean {

        String name;
        Integer va;
        public Bean(String name, Integer value) {
            this.name = name;
            this.va = value;
        }

        @Override
        public String toString() {
            return name +": " + va;
        }

    }
	
	public static void gc() {
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");
	}

    public static void main(String[] args) {
		Bean bean = new Bean("songli", 67);

		Bean[] referent = new Bean[20000]; 
		_UNSAFE.h2TagAndMoveRoot(referent, 1, 0);

		for (int i=0; i < referent.length; i++)
			referent[i] = new Bean("mybean:" + i, 100);// Throw Exception 

		System.out.println(referent[1000].toString());
		System.gc();

		Reference<Bean>[] soft_referent = new SoftReference[20000]; 
		for (int i=0; i < soft_referent.length; i++)
			soft_referent[i] = new SoftReference<Bean>(new Bean("mybean:" + i, 100)); 

		WeakReference<Bean>[] weakReference = new WeakReference[2000];
		_UNSAFE.h2TagAndMoveRoot(weakReference, 0, 0);

		System.gc();
	}
}
