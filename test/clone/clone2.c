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

//G() {
//    C();
//}

A() {
    C();
    //C();
}

B() {
    C();
}

C() {
    malloc(1);
    A();
}

//D() {
//    malloc(1);
//}

//C() {
//    D();
//    D();
//}
//
//D() {
//    E();
//    F();
//}

