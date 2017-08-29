//
// Created by tzhou on 6/29/17.
//

#include <iostream>
#include <cstdlib>
using namespace std;

class Base {
    int i;
public:
    virtual void hello()  { cout << "hello Base" << endl; }
    void bye()  { cout << "bye Base" << endl; }
};

class Derived: public Base {
    int i;
public:
    template <class T>
    void hello(T);

    void hello()  { cout << "hello Derived" << endl; }
};

template <class T>
void Derived::hello(T v) {
    cout << "hello " << v << endl;
}

int main() {
    Derived* d = new Derived();
    d->hello("from the outside");
    d->bye();

    Base* b = new Derived();
    b->hello();
    return 0;
}
