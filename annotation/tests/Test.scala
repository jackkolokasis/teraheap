import scala._
import java.lang._
import com.teraCache._
import java.lang.reflect.Method
import java.lang.reflect.Field

object Test {
   def main(args: Array[String]) {
      
      val str: String = new String("Hello World")
      val tera = new TeraCache

      val str3 = tera.test("lala")

      tera.sayHello()

      System.out.println(str3)
   }
}
