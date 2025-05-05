import java.lang.reflect.Field;

public class Phase2_GiveAddressesFromH2 {
  private static final sun.misc.Unsafe _UNSAFE;
  private static final int SIZE = 1000000;

  static class A {
    int x;
  }

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
    
    // Create an array of SIZE objects of class A
    A[] arrayOfA = new A[SIZE];

    // Instantiate SIZE objects of class A and assign them to the array
    for (int i = 0; i < SIZE; i++) {
      arrayOfA[i] = new A();
    }

    // Check that arrayOfA doesn't have an address from H2
    if (_UNSAFE.is_in_h2(arrayOfA))
      throw new RuntimeException("Object 'arrayOfA' shouldn't have an H2 address.");

    // Check that sub-object don't have an address from H2
    for (int i = 0; i < SIZE; i++) {
      if (_UNSAFE.is_in_h2(arrayOfA[i]))
        throw new RuntimeException("Object 'arrayOfA[" + i + "]' shouldn't have H2 address!");
    }

    // Mark to move in H2
	  _UNSAFE.h2TagAndMoveRoot(arrayOfA, 0, 0);

    // Check that arrayOfA don't have an address from H2
    if (_UNSAFE.is_in_h2(arrayOfA))
      throw new RuntimeException("After marking, object 'arrayOfA' shouldn't have an H2 address.");

    // Check that sub-object don't have an address from H2
    for (int i = 0; i < SIZE; i++) {
      if (_UNSAFE.is_in_h2(arrayOfA[i]))
        throw new RuntimeException("After marking, object 'arrayOfA[" + i + "]' shouldn't have H2 address!");
    }

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Check that arrayOfA has an address from H2
    if (!(_UNSAFE.is_in_h2(arrayOfA)))
      throw new RuntimeException("After GC, object 'arrayOfA' should have an H2 address.");

    // Check that sub-object have an address from H2
    for (int i = 0; i < SIZE; i++) {
      if (!(_UNSAFE.is_in_h2(arrayOfA[i])))
        throw new RuntimeException("After GC, object 'arrayOfA[" + i + "]' should have H2 address!");
    }

  }
}

