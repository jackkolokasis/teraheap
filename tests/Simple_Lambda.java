// Java program to demonstrate lambda expressions 
// to implement a user defined functional interface. 

// A sample functional interface (An interface with 
// single abstract method 
interface FuncInterface 
{ 
	// An abstract function 
	void abstractFun(int x); 

	// A non-abstract (or default) function 
	default void normalFun() 
	{ 
	System.out.println("Hello"); 
	} 
} 

class Simple_Lambda 
{ 
	public static void main(String args[]) 
	{ 
		// lambda expression to implement above 
		// functional interface. This interface 
		// by default implements abstractFun() 
		FuncInterface fobj = (int x)->System.out.println(2*x); 
		
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");

		// This calls above lambda expression and prints 10. 
		fobj.abstractFun(5); 
		fobj.abstractFun(1000); 
		
		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");


		System.out.println("=========================================");
		System.out.println("Call GC");
		System.gc();
		System.out.println("=========================================");
	} 
} 

