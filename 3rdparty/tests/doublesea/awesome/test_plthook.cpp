#include <functional>
#include <map>
#include <string>
#include <iostream>
#include "qdr_doctest.h"

#include "plthook.h"

#include "lib1.h"

extern "C" {

    /*
     * https://stackoverflow.com/questions/36692315/what-exactly-does-rdynamic-do-and-when-exactly-is-it-needed
     * The gcc -rdynamic option and the gcc -Wl,-E option are further synonyms for -Wl,--export-dynamic.
     *
     */

__attribute__((noinline))
int int_return_42() {
    return 42;
}

__attribute__((noinline))
int int_return_24() {
    return 24;
}
}


struct Spy {
    plthook_t *plthook;
    std::map<void *, std::string> map;
    Spy() {
        plthook_open(&plthook, nullptr);

        unsigned int pos = 0;
        const char * name;
        void ** addr;
        int rv;
        while ((rv = plthook_enum(plthook, &pos, &name, &addr)) == 0) {
            std::cout << name << " " << addr << std::endl;
        }
    }

    int spy(std::function<int()> f) {
        int ret = plthook_replace(plthook, "library_defined_function_c", (void *)&int_return_24, nullptr);
        printf("result: %d, %s\n", ret, plthook_error());
        return 0;
    }

//    int patch(std::function<int()> f) {
//        plthook_replace(this->plthook, "")
//        return 0;
//    }
};

static Spy cspy() {
    return Spy{};
}

//TEST_CASE("baf") {
//    CHECK(int_return_42() == 42);
//
//    auto lib = cspy();
//    auto fun = lib.spy(int_return_42);
//    CHECK(int_return_42() == 24);
//
////{
////    auto patch = lib.patch(int_return_24);
////    CHECK(int_return_42() == 24);
////}
//
////    CHECK(int_return_42() == 42);
//}

//TEST_CASE("lek") {
//    CHECK(library_defined_function_c() == 0xcafe);
//
//    auto lib = cspy();
//    auto fun = lib.spy(library_defined_function_c);
//    CHECK(library_defined_function_c() == 24);
//
////{
////    auto patch = lib.patch(int_return_24);
////    CHECK(int_return_42() == 24);
////}
//
////    CHECK(int_return_42() == 42);
//}

TEST_CASE("replace_by_addr") {
    printf("library_defined_function_c: %p\n", (void *)&library_defined_function_c);
    CHECK(library_defined_function_c() == 0xcafe);

    auto lib = cspy();
    plthook_replace_by_addr(lib.plthook, (void *)&library_defined_function_c, (void *)&int_return_24, nullptr);
    CHECK(library_defined_function_c() == 24);

//{
//    auto patch = lib.patch(int_return_24);
//    CHECK(int_return_42() == 24);
//}

//    CHECK(int_return_42() == 42);
}