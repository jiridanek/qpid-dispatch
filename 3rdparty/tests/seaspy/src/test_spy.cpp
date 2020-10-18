#include <functional>
#include <map>
#include <string>
#include <iostream>
#include "qdr_doctest.h"

#include "plthook.h"

extern "C" {
__attribute__((noinline))
int int_return_42() {
    return 42;
}

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
        int ret = plthook_replace(plthook, "int_return_42", (void *)&int_return_24, nullptr);
        printf("result: %d, %s\n", ret, plthook_error());
        return 0;
    }

    int patch(std::function<int()> f) {
//        plthook_replace(this->plthook, nullptr)
        return 0;
    }
};

static Spy cspy() {
    return Spy{};
}

TEST_CASE("baf") {
    CHECK(int_return_42() == 42);

    auto lib = cspy();
    auto fun = lib.spy(int_return_42);
//    CHECK(int_return_42() == 42);

    {
//        auto patch = lib.patch(int_return_24);
//        CHECK(int_return_42() == 24);
    }

//    CHECK(int_return_42() == 42);
}