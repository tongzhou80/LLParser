#include <iostream>
#include <omp.h>
int main() {
  #pragma omp parallel
  {
    std::cout << "Hello World\n";
  }
  return 0;
}
