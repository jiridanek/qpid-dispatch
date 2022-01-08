#include "qdr_doctest.h"
#include "lib1.h"

#include "Isolator.h"

int locally_defined_function() {
    return 0xcafe;
}

TEST_CASE ("locally_defined_function") {
    auto original_locally_defined_function = &locally_defined_function;
    {
        CHECK(locally_defined_function() == 0xcafe);
        FAKE_GLOBAL(locally_defined_function);
        CHECK(original_locally_defined_function == locally_defined_function);  // For some reason this holds
        CHECK(locally_defined_function() == 0);

        WHEN_CALLED(locally_defined_function()).Return(0xbabe);
        CHECK(original_locally_defined_function == locally_defined_function);  // ... even now
        CHECK(locally_defined_function() == 0xbabe);
    }
    ISOLATOR_CLEANUP();
    CHECK(original_locally_defined_function == locally_defined_function);
    CHECK(locally_defined_function() == 0xcafe);
}

TEST_CASE("Isolator") {

    SUBCASE("fake") {
        CHECK(library_defined_function_cpp() == 0xcafe);
        FAKE_GLOBAL(library_defined_function_cpp);

        CHECK(library_defined_function_cpp() == 0);
        WHEN_CALLED(library_defined_function_cpp()).Return(42);
        CHECK(library_defined_function_cpp() == 42);
    }
    ISOLATOR_CLEANUP();
    CHECK(library_defined_function_cpp() == 0xcafe);
}