import com.nvmUnsafe.NVMUnsafe;

public class Example {

    public static void main (String[] args) {
        NVMUnsafe tmp = new NVMUnsafe();

        int i = 12345;
        int retVal;
        long address;
        long allocAddr;

        address = tmp.nvmInitialPool("/mnt/pmem/file", 1024 * 1024);
        System.out.println("Initial Address " + address);

        allocAddr = tmp.nvmAllocateMemory(address, 4);
        System.out.println("Allocation Address " + allocAddr);

        tmp.putInt(i, address, allocAddr);

        retVal = tmp.getInt(address, allocAddr);

        System.out.println("Number is " + retVal);
    }
}
