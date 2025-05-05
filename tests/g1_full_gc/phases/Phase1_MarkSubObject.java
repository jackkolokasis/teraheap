import java.lang.reflect.Field;
import java.util.ArrayList;

public class Phase1_MarkSubObject {
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
    // Create an ArrayList of Arrays of class A
    ArrayList<A[]> arraysOfA = new ArrayList<A[]>();
    // Create an array of objects of class A
    A[] arrayOfA = new A[SIZE];
    // Create an array of object of class A what will not be marked.
    A[] arrayOfA_nomove = new A[SIZE];

    // Instantiate objects of class A and assign them to the array
    for (int i = 0; i < SIZE; i++) {
      arrayOfA[i] = new A();
      arrayOfA_nomove[i] = new A();
    }

    arraysOfA.add(arrayOfA);
    arraysOfA.add(arrayOfA_nomove);

    // Mark to move in H2
	  _UNSAFE.h2TagAndMoveRoot(arraysOfA.get(0), 0, 0);

    // Check that the first array is marked
    if (!(_UNSAFE.is_marked_move_h2(arraysOfA.get(0)) && _UNSAFE.is_marked_move_h2(arrayOfA)))
      throw new RuntimeException("Object 'arraysOfA[0]' should be marked!");

    // Check that the second array is not marked
    if (_UNSAFE.is_marked_move_h2(arraysOfA.get(1)) || _UNSAFE.is_marked_move_h2(arrayOfA_nomove))
      throw new RuntimeException("Object 'arraysOfA[1]' shouldn't be marked!");

    // Also, the ArrayList (parent) shouldn't be marked.
    if (_UNSAFE.is_marked_move_h2(arraysOfA))
      throw new RuntimeException("Object 'arraysOfA' shouldn't be marked!");

    // Check that first array is not moved to H2 yet.
    if (_UNSAFE.is_in_h2(arraysOfA.get(0)))
      throw new RuntimeException("GC is not yet triggered. Object 'arraysOfA[0]' shouldn't be in h2!");

    // -------------------------------------------
    // Trigger a Full GC
    System.gc();
    // -------------------------------------------

    // Check that the first array is in H2
    if (!(_UNSAFE.is_in_h2(arrayOfA) && _UNSAFE.is_in_h2(arraysOfA.get(0))))
      throw new RuntimeException("After GC, object 'arraysOfA[0]' should be in h2!");

    // Check that the second array is not moved to H2.
    if (_UNSAFE.is_in_h2(arrayOfA_nomove) || _UNSAFE.is_in_h2(arraysOfA.get(1)))
      throw new RuntimeException("After GC, object 'arraysOfA[1]' shouldn't be in h2!");

    // Check that arraysOfA is not moved to H2.
    if (_UNSAFE.is_in_h2(arraysOfA))
      throw new RuntimeException("After GC, object 'arraysOfA' shouldn't be in h2!");

    // Check that all objects of arrayOfA are moved to H2,
    // and objects of arrayOfA_nomove are not!
    for (int i = 0; i < SIZE; i++) {
      if (!(_UNSAFE.is_in_h2(arrayOfA[i]) && _UNSAFE.is_in_h2(arraysOfA.get(0)[i])))
        throw new RuntimeException("Object 'arrayOfA[" + i + "]' should be in H2!");
      if (_UNSAFE.is_in_h2(arrayOfA_nomove[i]) || _UNSAFE.is_in_h2(arraysOfA.get(1)[i]))
        throw new RuntimeException("Object 'arrayOfA_nomove[" + i + "]' shouldn't be in H2!");
    }
  }
}

