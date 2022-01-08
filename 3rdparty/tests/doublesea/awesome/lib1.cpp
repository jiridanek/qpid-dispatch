//
// Created by jdanek on 10/2/20.
//

#include "lib1.h"

extern "C" {
int library_defined_function_c() {
    return 0xcafe;
}
}
int library_defined_function_cpp() {
    return 0xcafe;
}