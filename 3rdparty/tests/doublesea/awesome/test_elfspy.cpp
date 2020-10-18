extern "C" {
#include <stdlib.h>
}

#include "qdr_doctest.h"

#include "elfspy/SPY.h"
#include "elfspy/Call.h"
#include "elfspy/Arg.h"
#include "elfspy/Result.h"
#include "elfspy/Fake.h"


int locally_defined_function() {
    return 0xcafe;
}

// TODO check library gives actionable failures if it fails to mock, or if proceeds silently
//(elfspy prints to stdout "cannot find definition of function malloc")

void setup() {
    char *argv[] = {(char*)"test_elfspy", nullptr};
    spy::initialise(1, argv);
}

TEST_CASE ("locally_defined_function") {
        setup();
    {
                CHECK(locally_defined_function() == 0xcafe);
        auto spy = SPY(locally_defined_function);
                CHECK(locally_defined_function() == 0xcafe);
        auto fake = spy::fake(spy, [](auto ...) { return 0xbabe; });
                CHECK(locally_defined_function() == 0xbabe);
    }
            CHECK(locally_defined_function() == 0xcafe);
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