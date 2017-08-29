//
// Created by tlaber on 6/21/17.
//

#include <stdio.h>

typedef void (*function_ptr) (int);

void foo(int i) {
    printf("foo %d", i);
}

void bar(int i) {
    printf("bar %d", i);
}

void test(function_ptr p, int i) {
    if (i == 1) {
        p = foo;
    }
    p(i);
}

int main(int argc, char** argv) {
    void (*p)(int );
    if (argc == 2) {
        p = foo;
    }
    else {
        p = bar;
    }

    p(2);

    void* q = p;

}