extern "C" {
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
}
#include <string>

#include "qdr_doctest.h"

#include "elfspy/SPY.h"
#include "elfspy/Call.h"
#include "elfspy/Arg.h"
#include "elfspy/Result.h"
#include "elfspy/Fake.h"

#include "lib1.h"

std::string getpath() {
    char buf[PATH_MAX + 1];
    if (readlink("/proc/self/exe", buf, sizeof(buf) - 1) == -1) {
        throw std::string("readlink() failed");
    }
    std::string str(buf);
    return str;
}

extern "C" {
int locally_defined_function() {
    return 0xcafe;
}
}

// TODO check library gives actionable failures if it fails to mock, or if proceeds silently
//(elfspy prints to stdout "cannot find definition of function malloc")

void setup() {
    auto name = getpath();
    char *argv[] = {const_cast<char*>(name.c_str()), nullptr};
    spy::initialise(1, argv);
}

TEST_CASE ("locally_defined_function") {
    setup();
    auto original_locally_defined_function = &locally_defined_function;
    {
        CHECK(locally_defined_function() == 0xcafe);
        auto spy = SPY(locally_defined_function);
        CHECK(locally_defined_function() == 0xcafe);

        auto fake = spy::fake(spy, [](auto ...) { return 0xbabe; });
        CHECK(original_locally_defined_function != locally_defined_function);
        CHECK(locally_defined_function() == 0xbabe);
    }
    CHECK(original_locally_defined_function == locally_defined_function);
    CHECK(locally_defined_function() == 0xcafe);
}

extern "C" {
    void test_library_defined_function_cpp() {
        auto original_library_defined_function_cpp = &library_defined_function_cpp;
        {
            CHECK(library_defined_function_cpp() == 0xcafe);
            auto spy = SPY(library_defined_function_cpp);
            CHECK(library_defined_function_cpp() == 0xcafe);

            auto fake = spy::fake(spy, [](auto ...) { return 0xbabe; });
            CHECK(original_library_defined_function_cpp != library_defined_function_cpp);
            CHECK(library_defined_function_cpp() == 0xbabe);
        }
        CHECK(original_library_defined_function_cpp == library_defined_function_cpp);
        CHECK(library_defined_function_cpp() == 0xcafe);
    }
}

TEST_CASE ("library_defined_function_cpp_body_in_func") {
    setup();
    test_library_defined_function_cpp();
}

TEST_CASE ("library_defined_function_cpp") {
    setup();
    auto original_library_defined_function_cpp = &library_defined_function_cpp;
    {
        CHECK(library_defined_function_cpp() == 0xcafe);
        auto spy = SPY(library_defined_function_cpp);
        CHECK(library_defined_function_cpp() == 0xcafe);

        auto fake = spy::fake(spy, [](auto ...) { return 0xbabe; });
        CHECK(original_library_defined_function_cpp != library_defined_function_cpp);
        CHECK(library_defined_function_cpp() == 0xbabe);
    }
    CHECK(original_library_defined_function_cpp == library_defined_function_cpp);
    CHECK(library_defined_function_cpp() == 0xcafe);
}

void * stub_malloc(long unsigned int size) {
    return (void *)42;
}

TEST_CASE ("locally_used_malloc") {
    setup();

    auto original_malloc = &malloc;
    {
        CHECK(original_malloc == malloc);
        auto spy = SPY(malloc);
        CHECK(original_malloc != malloc);
        auto fake = spy::fake(spy, &stub_malloc);
        CHECK(original_malloc != malloc);
        CHECK(malloc(0) == (void *) 42);
    }
    CHECK(original_malloc == malloc);
}