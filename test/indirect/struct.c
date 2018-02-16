//
// Created by tlaber on 6/21/17.
//
#include <stdio.h>

typedef void (*function_ptr) ();

typedef struct {
    void (* p) () ;
    int n;
} apple ;

typedef struct {
  void (* p) (int, ...) ;
    int n;
} orange ;

void foo () {
    printf("foo");
}

function_ptr get_fp(apple* a) {
    return a->p;
}

void test(apple* a) {
    //if (a->p == NULL) {
        a->p = foo;
    //}
    get_fp(a)();
}

int main ( int argc , char ** argv ) {
    /* apple *a = malloc ( sizeof ( apple )) ; */
    /* a ->p = foo ; */
    /* a->p(); */
    /* orange o; */
   apple a;
   a.p = & foo ;

   //apple b = a;
   a.p () ;
   return 0;
}
