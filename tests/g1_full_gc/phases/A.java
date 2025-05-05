public class A {
  int x;

  public A() {
    this.x = 0;
  }

  public A(int x) {
    this.x = x;
  }

  @Override
  public String toString() {
    return getClass().getSimpleName() + " { x=" + this.x +" }";
  }
}
