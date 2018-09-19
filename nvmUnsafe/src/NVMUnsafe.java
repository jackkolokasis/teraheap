import java.lang.instrument.Instrumentation;

public class NVMUnsafe {
   static {
      System.loadLibrary("NVMUnsafe"); // Load native library at runtime
                                       // libNVMUnsafe.so (Unix)
   }
 
   // Declare a native method sayHello() that receives nothing and returns void
   private native long   nvmInitialPool(String path, long pool_size);
   private native long   nvmAllocateMemory(long pmp, long size);
   private native int    getInt(long pmp, long offset);
   private native void   putInt(int number, long pmp, long offset);
   private native void   putLong(long number, long pmp, long offset);
   private native long   getLong(long pmp, long offset);
   private native void   putDouble(long number, long pmp, long offset);
   private native double getDouble(long pmp, long offset);


  // private native void setObject(Object obj, long pmp, long offset);
   //private native long nvm_free(long pmp, long offset);
   
   public static void main(String[] args)
   {
       NVMUnsafe tmp = new NVMUnsafe();

       int i = 12345;
       int retVal;
       long address;
       long allocAddr;

       address = tmp.nvmInitialPool("/mnt/mem/file", 1024 * 1024);
       System.out.println("Initial Address " + address);

       allocAddr = tmp.nvmAllocateMemory(address, 4);
       System.out.println("Allocation Address " + allocAddr);

       tmp.putInt(i, address, allocAddr);

       retVal = tmp.getInt(address, allocAddr);

       System.out.println("Number is " + retVal);
   }
}
