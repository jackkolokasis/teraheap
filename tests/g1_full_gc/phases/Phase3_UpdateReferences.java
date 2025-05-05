import java.lang.reflect.Field;
import java.util.ArrayList;

public class Phase3_UpdateReferences {
  private static final sun.misc.Unsafe _UNSAFE;
  private static final int SIZE = 200000;
  private static final int SMALL_SIZE = 2048;

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
    // Create an ArrayList of Arrays of class A
    ArrayList<A[]> arrays = new ArrayList<A[]>();
    // Create an array of SIZE objects of class A
    A[] arrayOfA = new A[SIZE];
    // Create an array of SMALL_SIZE objects of class A
    A[] smallArrayOfA = new A[SMALL_SIZE];

    arrays.add(arrayOfA);
    arrays.add(smallArrayOfA);

    // Instantiate SIZE objects of class A and assign them to the array
    for (int i = 0; i < SIZE; i++) {
      arrayOfA[i] = new A();
      arrayOfA[i].x = i;
    }

    // Instantiate SMALL_SIZE objects of class A and assign them to the array
    for (int i = 0; i < SMALL_SIZE; i++) {
      smallArrayOfA[i] = new A();
      smallArrayOfA[i].x = i;
    }

    // Check that arrayOfA and sub-objects are accessible before GC 
    for (int i = 0; i < SIZE; i++) {
      if (arrays.get(0)[i].x != i)
        throw new RuntimeException("Object 'arrayOfA[" + i + "]' should be A { x=" + i + " }! Found: " + arrays.get(0)[i]);
    }

    // Check that smallArrayOfA and sub-objects are accessible before GC 
    for (int i = 0; i < SMALL_SIZE; i++) {
      if (arrays.get(1)[i].x != i)
        throw new RuntimeException("Object 'smallArrayOfA[" + i + "]' should be A { x=" + i + " }! Found: " + arrays.get(1)[i]);
    }

    // Mark to move in H2
	  _UNSAFE.h2TagAndMoveRoot(arrayOfA, 0, 0);
	  _UNSAFE.h2TagAndMoveRoot(smallArrayOfA, 0, 0);

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Check that objects are in H2.
    if (_UNSAFE.is_in_h2(arrays))
      throw new RuntimeException("After GC, object 'arrays' shouldn't be in H2.");

    if (!(_UNSAFE.is_in_h2(arrayOfA)))
      throw new RuntimeException("After GC, object 'arrayOfA' should be in H2.");

    if (!(_UNSAFE.is_in_h2(smallArrayOfA)))
      throw new RuntimeException("After GC, object 'smallArrayOfA' should be in H2.");

    for (int i = 0; i < SIZE; i++) {
      if (!(_UNSAFE.is_in_h2(arrayOfA[i])))
        throw new RuntimeException("After GC, object 'arrayOfA[" + i + "]' should be in H2!");
    }

    for (int i = 0; i < SMALL_SIZE; i++) {
      if (!(_UNSAFE.is_in_h2(smallArrayOfA[i])))
        throw new RuntimeException("After GC, object 'arrayOfA[" + i + "]' should be in H2!");
    }

    // Check Forward References

    // Check that arrayOfA and sub-objects are accessible after GC 
    for (int i = 0; i < SIZE; i++) {
      if (arrays.get(0)[i].x != i)
        throw new RuntimeException("After GC object 'arrayOfA[" + i + "]' should be A { x=" + i + " }! Found: " + arrays.get(0)[i]);
    }

    // Check that smallArrayOfA and sub-objects are accessible after GC 
    for (int i = 0; i < SMALL_SIZE; i++) {
      if (arrays.get(1)[i].x != i)
        throw new RuntimeException("After GC object 'smallArrayOfA[" + i + "]' should be A { x=" + i + " }! Found: " + arrays.get(1)[i]);
    }

    // Create a backward reference.
    A new_object = new A();
    new_object.x = 481516;

    arrayOfA[5] = new_object;
    smallArrayOfA[5] = new_object;

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Check that objects are in H2.
    if (_UNSAFE.is_in_h2(arrays))
      throw new RuntimeException("After GC, object 'arrays' shouldn't be in H2.");

    if (_UNSAFE.is_in_h2(new_object)
        || _UNSAFE.is_in_h2(arrayOfA[5])
        || _UNSAFE.is_in_h2(smallArrayOfA[5])
        || _UNSAFE.is_in_h2(arrays.get(0)[5])
        || _UNSAFE.is_in_h2(arrays.get(1)[5]))
      throw new RuntimeException("After GC, object 'new_object' shouldn't be in H2.");

    if (!(_UNSAFE.is_in_h2(arrayOfA)))
      throw new RuntimeException("After GC, object 'arrayOfA' should be in H2.");

    if (!(_UNSAFE.is_in_h2(smallArrayOfA)))
      throw new RuntimeException("After GC, object 'smallArrayOfA' should be in H2.");

    for (int i = 0; i < SIZE; i++) {
      if (i == 5) continue;
      if (!(_UNSAFE.is_in_h2(arrayOfA[i])))
        throw new RuntimeException("After GC, object 'arrayOfA[" + i + "]' should be in H2!");
    }

    for (int i = 0; i < SMALL_SIZE; i++) {
      if (i == 5) continue;
      if (!(_UNSAFE.is_in_h2(smallArrayOfA[i])))
        throw new RuntimeException("After GC, object 'arrayOfA[" + i + "]' should be in H2!");
    }

    // Check Forward and Backward References

    // Check that arrayOfA and sub-objects are accessible after GC 
    for (int i = 0; i < SIZE; i++) {
      if (i == 5) {
        if (arrays.get(0)[i].x != 481516)
          throw new RuntimeException("After GC object 'arrayOfA[" + i + "]' should be A { x=481516 }! Found: " + arrays.get(0)[i]);
        continue;
      }
      if (arrays.get(0)[i].x != i)
        throw new RuntimeException("After GC object 'arrayOfA[" + i + "]' should be A { x=" + i + " }! Found: " + arrays.get(0)[i]);
    }

    // Check that smallArrayOfA and sub-objects are accessible after GC 
    for (int i = 0; i < SMALL_SIZE; i++) {
      if (i == 5) {
        if (arrays.get(1)[i].x != 481516)
          throw new RuntimeException("After GC object 'arrayOfA[" + i + "]' should be A { x=481516 }! Found: " + arrays.get(1)[i]);
        continue;
      }
      if (arrays.get(1)[i].x != i)
        throw new RuntimeException("After GC object 'arrayOfA[" + i + "]' should be A { x=" + i + " }! Found: " + arrays.get(1)[i]);
    }
  }
}
