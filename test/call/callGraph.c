//
// Created by tlaber on 6/21/17.
//


A();
B();
C();
D();
E();
F();

A() {
    C();
}

B() {
    C();
}

C() {
    D();
    D();
}

D() {
    E();
    F();
}
