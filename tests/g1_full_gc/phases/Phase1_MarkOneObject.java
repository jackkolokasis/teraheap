import java.lang.reflect.Field;

public class Phase1_MarkOneObject {
  private static final sun.misc.Unsafe _UNSAFE;
  private static final int SIZE = 2048;

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
    // Create an array of objects of class A
    A[] arrayOfA = new A[SIZE];
    // Create an array of object of class A what will not be marked.
    A[] arrayOfA_nomove = new A[SIZE];

    // Instantiate objects of class A and assign them to the array
    for (int i = 0; i < SIZE; i++) {
      arrayOfA[i] = new A();
      arrayOfA_nomove[i] = new A();
    }

    // Mark to move in H2
	  _UNSAFE.h2TagAndMoveRoot(arrayOfA, 0, 0);
    
    // Check that arrayOfA is marked
    if (!(_UNSAFE.is_marked_move_h2(arrayOfA)))
      throw new RuntimeException("Object 'arrayOfA' should be marked!");

    // Check that sub objects of arrayOfA are not marked
    if (_UNSAFE.is_marked_move_h2(arrayOfA[5]))
      throw new RuntimeException("GC is not triggered. Object 'arrayOfA[x]' shouldn't be marked!");

    // Check that arrayOfA is not moved to H2 yet.
    if (_UNSAFE.is_in_h2(arrayOfA))
      throw new RuntimeException("GC is not yet triggered. Object 'arrayOfA' shouldn't be in h2!");

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Check that arrayOfA is in H2
    if (!(_UNSAFE.is_in_h2(arrayOfA)))
      throw new RuntimeException("After GC, object 'arrayOfA' should be in h2!");

    // Check that arrayOfA_nomove is not moved to H2.
    if (_UNSAFE.is_in_h2(arrayOfA_nomove))
      throw new RuntimeException("After GC, object 'arrayOfA_nomove' shouldn't be in h2!");

    // and objects of arrayOfA_nomove are not!
    for (int i = 0; i < SIZE; i++) {
      if (!(_UNSAFE.is_in_h2(arrayOfA[i])))
        throw new RuntimeException("After GC, object 'arrayOfA[" + i + "]' should be in h2!");
      if (_UNSAFE.is_in_h2(arrayOfA_nomove[i]))
        throw new RuntimeException("After GC, object 'arrayOfA_nomove[" + i + "]' shouldn't be in h2!");
    }
  }
}

