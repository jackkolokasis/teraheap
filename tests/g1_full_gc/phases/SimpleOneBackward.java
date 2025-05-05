import java.lang.reflect.Field;

public class SimpleOneBackward {
  
  private static final sun.misc.Unsafe _UNSAFE;
  private static final int SIZE = 1;

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
    A[] arr = new A[SIZE];

    for (int i = 0; i < SIZE; i++) {
      arr[i] = new A(i);
    }

    _UNSAFE.h2TagAndMoveRoot(arr, 0, 0);

    // Verify that all objects are correct after marking.
    for (int i = 0; i < SIZE; i++) {
      if (arr[i].x != i)
        throw new RuntimeException("Object 'arr[" + i + "]' should be A { x=" + i + "} found " + arr[i]);
    }

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Verify that all objects are correct after GC.
    for (int i = 0; i < SIZE; i++) {
      if (arr[i].x != i)
        throw new RuntimeException("Object 'arr[" + i + "]' should be A { x=" + i + "} found " + arr[i]);
    }

    // Create a backward pointer
    A a = new A(18);

    arr[SIZE - 1] = a;

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Verify that new object is not transfered in H2.
    if (_UNSAFE.is_in_h2(a) || _UNSAFE.is_in_h2(arr[SIZE - 1]))
      throw new RuntimeException("Backward reference transfered to H2.");

    // Verify that all objects are correct after 2nd GC.
    for (int i = 0; i < SIZE - 1; i++) {
      if (arr[i].x != i)
        throw new RuntimeException("Object 'arr[" + i + "]' should be A { x=" + i + "} found " + arr[i]);
    }

    // Verify backward reference
    if (arr[SIZE - 1].x != 18)
      throw new RuntimeException("Backward reference break. Last object should be A { x=18 } found " + arr[SIZE - 1]);
  }
}
