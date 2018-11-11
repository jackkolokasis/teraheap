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
        long address;
        long allocAddr;

        address = tmp.nvmInitialPool("/mnt/pmemdir/file", 1073741824);
        System.out.println("Initial Address " + address);

        allocAddr = tmp.nvmAllocateMemory(address, 4);
        System.out.println("Allocation Address " + allocAddr);

        tmp.putInt(i, address, allocAddr);

        retVal = tmp.getInt(address, allocAddr);

        System.out.println("Number is " + retVal);

        tmp.nvmFreeMemory(address, allocAddr);
        
        System.out.println("Free Memory");
    }
}
