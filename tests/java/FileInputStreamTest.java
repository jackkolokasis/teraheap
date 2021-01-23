package com.tutorialspoint;

import java.io.IOException;
import java.io.FileInputStream;

public class FileInputStreamTest {
   public static void main(String[] args) throws IOException {
      FileInputStream fis = null;
      int i = 0;
      char c;
      
      try {
         // create new file input stream
         fis = new FileInputStream("test.txt");
         
         // read byte from file input stream
         i = fis.read();
         
         // convert integer from character
         c = (char)i;
         
         // print character
         System.out.println(c);
         
         // close file input stream
         fis.close();
         System.out.println("Close() invoked");
         
         // tries to read byte from close file input stream
         i = fis.read();
         c = (char)i;
         System.out.println(c);
         
      } catch(Exception ex) {
         // if an I/O error occurs
         System.out.println("IOException: close called before read()");
      } finally {
         // releases all system resources from the streams
         if(fis!=null) {
            fis.close();
         }
      }
   }
}
