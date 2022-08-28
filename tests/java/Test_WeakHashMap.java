import java.util.HashMap;  
import java.util.Iterator;  
import java.util.Map;  
import java.util.WeakHashMap;  
  
import java.lang.reflect.Field;

public class Test_WeakHashMap {  
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

    public static void main(String[] args) throws Exception {  
        String a = new String("a");  
        String b = new String("b");  
        Map weakmap = new WeakHashMap();  

		// Mark this weakmap to be moved in H2
		_UNSAFE.h2TagAndMoveRoot(weakmap, 0, 0);

        Map map = new HashMap();  
        map.put(a, "aaa");  
        map.put(b, "bbb");  
  
        System.gc();  
          
        weakmap.put(a, "aaa");  
        weakmap.put(b, "bbb");  
          
        map.remove(a);  
          
        a=null;  
        b=null;  
          
        System.gc();  
        Iterator i = map.entrySet().iterator();  
        while (i.hasNext()) {  
            Map.Entry en = (Map.Entry)i.next();  
            System.out.println("map:"+en.getKey()+":"+en.getValue());  
        }  
  
        Iterator j = weakmap.entrySet().iterator();  
        while (j.hasNext()) {  
            Map.Entry en = (Map.Entry)j.next();  
            System.out.println("weakmap:"+en.getKey()+":"+en.getValue());  
              
        }  
    }  
}       
