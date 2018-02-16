//
// Created by tlaber on 6/21/17.
//

#include <stdio.h>

typedef void (*func_ptr) (int);
func_ptr gfp;

void foo(int i) {
    printf("foo %d", i);
}

void bar(int i) {
    printf("bar %d", i);
}

void test(func_ptr p, int i) {
    if (i == 1) {
        p = foo;
    }
    p(i);
}

int main(int argc, char** argv) {
  func_ptr p, q;
  p = bar;
  gfp = foo;
  gfp(1);
  p(2);

  q = p;
  q(2);  
}
