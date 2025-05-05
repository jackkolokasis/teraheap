import java.lang.reflect.Field;

public class SimpleOneObj {
  
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

  public static void main(String[] args) {
    A a = new A();
    a.x = 4;

    int the_hash = a.hashCode();

    _UNSAFE.h2TagAndMoveRoot(a, 0, 0);

    if (the_hash != a.hashCode())
      throw new RuntimeException("Object 'a' has different hash after marking!");
    if (a.x != 4)
      throw new RuntimeException("Object 'a' has different x after marking!");

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    if (the_hash != a.hashCode())
      throw new RuntimeException("Object 'a' has different hash after GC!");
    if (a.x != 4)
      throw new RuntimeException("Object 'a' has different x after GC!");
  }
}
