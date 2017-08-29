//
// Created by tlaber on 6/25/17.
//
#include <stdlib.h>

A();
B();
C();
D();
E();
F();
G();
H();
I();

A() {
    C();
    C();
}

B() {
    C();
}

C() {
    D();
}

D() {
    malloc(1);
}

main() {
    A();
    B();
}

//C() {
//    D();
//    D();
//}
//
//D() {
//    E();
//    F();
//}

