import com.nvmUnsafe.*;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;

public class Example {
    

    public static void main (String[] args) {
        com.nvmUnsafe.NVMUnsafe tmp;

        try {
            Field nvmUnsafeField = NVMUnsafe.class.getDeclaredField("theNVMUnsafe");
            nvmUnsafeField.setAccessible(true);
            tmp = (com.nvmUnsafe.NVMUnsafe) nvmUnsafeField.get(null);
        } catch (Throwable cause) {
            tmp = null;
        }

        int i = 12345;
        int retVal;
        long allocAddr;

        tmp.nvmInitialPool("/mnt/pmemdir/", 1073741824);

        allocAddr = tmp.nvmAllocateMemory(4);
        System.out.println("Allocation Address " + allocAddr);

        tmp.putInt(i, allocAddr);

        retVal = tmp.getInt(allocAddr);

        System.out.println("Number is " + retVal);

        tmp.nvmFreeMemory(allocAddr);
         
        System.out.println("Free Memory");

        tmp.nvmDelete();
        System.out.println("Delete Memory");
    }
}
