import scala._
import java.lang._
import com.teraCache._
import java.lang.reflect.Method
import java.lang.reflect.Field

class Person(var firstName: String, val lastName: String) {

    println("the constructor begins")
    val fullName = firstName + " " + lastName

    // define some methods
    def foo { println("foo") }
    def printFullName {
        // access the fullName field, which is created
        // above
        println(fullName) 
    }

    printFullName
    println("still in the constructor")
}

object AllocateClass {
   def main(args: Array[String]) {
      
      val str: String = new String("Hello World")
      val tera = new TeraCache

      var p: Person = tera.test_class("jack", "kolokasis").asInstanceOf[Person]
      p.printFullName

      val test = new Person("test", "tester")

      println(test.firstName)

   }
}
