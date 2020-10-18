# Double Sea

_Test Doubles for the World of C/C++_

## Features

binary, lib1.so, lib2.so

## Hello World

char * world() {
    return "World";
}

int main() {
    // unadulterated call to the world function
    printf("Hello, %s!", world());
    //
    [](){return "Wolf"};

    // the world function is now redefined
    printf("Hello, %s!", world());
}

## Awesome GOT/PLT-based function interception

There is many other libraries which work by intercepting function calls in the GOT/PLT dynamic linking mechanism.
Each has some disadvantages.

Closed-source,
Header file doesn't compile as C++.
Tied to Google Test.
