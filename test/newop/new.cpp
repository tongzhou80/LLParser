
class A {
  
};

int main() {
  int* pi = new int[1];
  char* pc = new char[1];
  int* pi1 = new int;
  long* pl = new long;
  A* pa = new A();

  delete[] pi;
  delete[] pc;
  delete pi1;
  delete pl;
  delete pa;
}
