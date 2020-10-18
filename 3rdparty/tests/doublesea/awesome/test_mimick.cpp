extern "C" {
#include <stdlib.h>
#include "mimick.h"
}

#include "qdr_doctest.h"
//#include "../../mimick/src/mock.h"
//#include "../../mimick/src/core.h"
//#include "../../mimick/include/mimick.h"

void * fake_malloc(unsigned long int size) {
    return (void *) 42;
}

TEST_CASE("mimick_malloc") {
    auto original_malloc = &malloc;

//    mmk_init();  // not necessary
    struct mmk_mock_options opts = {1, 0};
    mmk_mock_create_internal("malloc@self", (mmk_fn) &fake_malloc, opts);
    CHECK(original_malloc != &malloc);

    CHECK(malloc(0) == (void *) 42);
    mmk_reset(malloc);
    CHECK(original_malloc == &malloc);
}
