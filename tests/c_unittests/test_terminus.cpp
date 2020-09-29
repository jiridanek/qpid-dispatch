/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <iostream>
#include <plthook/plthook.h>
#include "qdr_doctest.hpp"
extern "C" {
//#include "mimick.h"
#include "plthook.h"
}
extern "C" {
#include <qpid/dispatch/amqp.h>
}

#include "elfspy/SPY.h"
#include "elfspy/Call.h"
#include "elfspy/Arg.h"
#include "elfspy/Result.h"

extern "C" {
#include "qpid/dispatch/router_core.h"

#include <../src/router_core/router_core_private.h>
#include <../src/terminus_private.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
}
#include "elfspy/Fake.h"

int fake_vsnprintf(char * s, size_t n, const char * format, va_list arg);
int fake_snprintf(char * s, size_t n, const char * format) {
    printf("fake snprintf\n");
}

__attribute__((noinline))
int add(int a, int b)
{
  std::cout << "add(" << a << ", " << b << ") -> " << a + b << std::endl;
  return a + b;
}

__attribute__((noinline))
int f()
{
    int rv = 0;
    rv += add(1, 2);
    rv += add(3, 4);
    return rv;
}

int fake(const char *) {
    return 42;
}

int fake2(const char *name, const char *proto) {
    return 42;
}

int my_snprintf(char *__s, size_t __maxlen, const char *__format, ...) {
    return -1;
}

void * fake_malloc(unsigned long size) {
    printf("from fake malloc\n");
    return reinterpret_cast<void *>(33);
}

//template <typename H, typename ReturnType, typename... ArgTypes>
//inline auto fake(ReturnType (*patch)(ArgTypes...)) -> spy::Fake<H, ReturnType, ArgTypes...>
//{
//    H h = SPY(&vsnprintf);
//    return { h, patch };
//}

int int_return_42() {
    return 42;
}

int int_return_24() {
    return 24;
}

TEST_CASE ("plthook defined here") {
    printf("&int_return_42: %p\n", &int_return_42);
    plthook_t *plthook;
    {
        if(plthook_open(&plthook, nullptr) != 0) {
            printf("error: %s\n", plthook_error());
        }
        void *oldfunc;
        plthook_replace(plthook, "int_return_42", (void *)(&int_return_24), &oldfunc);
        printf("&int_return_42: %p\n", &int_return_42);
        plthook_replace(plthook, "int_return_42", oldfunc, nullptr);
    }
    plthook_close(plthook);

    printf("&int_return_42: %p\n", &int_return_42);
}


TEST_CASE ("self_contained_mallocPlthook") {
    printf("&malloc: %p\n", &snprintf);

//    snprintf(nullptr, 0, "baf");
    plthook_t *plthook;
    {
        if(plthook_open(&plthook, nullptr) != 0) {
            printf("error: %s\n", plthook_error());
        }
        void *oldfunc;
        plthook_replace(plthook, "snprintf", (void *)(&fake_snprintf), &oldfunc);
        printf("&snprintf: %p\n", &snprintf);
        printf("%p\n", snprintf("", 42, nullptr));
        plthook_replace(plthook, "snprintf", oldfunc, nullptr);
    }
    plthook_close(plthook);
    printf("%p\n", snprintf("", 0, nullptr));

    printf("&snprintf: %p\n", &snprintf);
}

TEST_CASE ("self_contained_malloc") {
    char *argv[2] = {(char *) "c_unittests", nullptr};
    spy::initialise(1, argv);

    printf("&malloc: %p\n", &malloc);

    {
        auto mlc = SPY(&malloc);
        auto mlc_call = spy::call(mlc);
//    auto fake2 = spy::fake(malloc, &malloc);

        char *buf = (char *) malloc(30);
        printf("&malloc: %p\n", &malloc);
        sprintf(buf, "lek");
        printf("%s", buf);

                CHECK(mlc_call.count() == 1);
    }

    printf("&malloc: %p\n", &malloc);
}

TEST_CASE("self_contained_vsnprintf") {
    char* argv[2] = {(char*)"c_unittests", nullptr};
    spy::initialise(1, argv);

    printf("&vsnprintf: %p\n", &vsnprintf);

    {
        auto vsnprintf_ = SPY(&vsnprintf);
        auto fake2 = spy::fake(vsnprintf_, &fake_vsnprintf);

        char buf[30];
        vsnprintf(buf, 30, "lek", NULL);
        printf("&vsnprintf: %p\n", &vsnprintf);
    }

    printf("&vsnprintf: %p\n", &vsnprintf);
}

TEST_CASE("localfunc") {
    char* argv[2] = {(char*)"c_unittests", nullptr};
    spy::initialise(1, argv);

    auto vsnprintf_ = SPY(&vsnprintf);
    auto fake2 = spy::fake(vsnprintf_, &fake_vsnprintf);

//    auto snprintf_ = SPY(&snprintf);
//    auto add_call = spy::call(snprintf_);   // capture number of calls to add()

    auto malloc_ = SPY(&malloc);
    auto malloc_call = spy::call(malloc_);   // capture number of calls to add()

//    auto fake2 = spy::fake(snprintf_, &my_snprintf);

//    auto snprintf_ = SPY(&vsnprintf);
//    auto fake3 = spy::fake(snprintf_, &my_snprintf);

//    f2();

//    CHECK(add_call.count() == 1);  // verify add is called twice
    CHECK(malloc_call.count() == 1);  // verify add is called twice
    CHECK(&fake2 != nullptr);
}

int fake_vsnprintf(char * s, size_t n, const char * format, va_list arg) {
    return -1;
}

TEST_CASE("safe_snprintf_vsnprintf_failed") {
    const int   OUTPUT_SIZE = 128;
    const char *TEST_MESSAGE = "something";
    const int   LEN = strlen(TEST_MESSAGE);
    size_t len;
    char output[OUTPUT_SIZE];

    // weird elfspy boilerplate
    char* argv[2] = {(char*)"c_unittests", nullptr};
    spy::initialise(0, argv);

    // setup the fake, it will be unset when the variables go out of scope
    auto vsnprintf_ = SPY(&vsnprintf);
     auto vsnprintf_fake = spy::fake(vsnprintf_, &fake_vsnprintf);
    // alternative way for previous line:
    // auto vsnprintf_fake = spy::fake(vsnprintf_, [](auto ...) { return -1; });  // I can haz lambdaz

    // run the test, simulating a failed vsnprintf
    output[0] = 'a';
    len = safe_snprintf(output, LEN+10, TEST_MESSAGE);
    CHECK(0 == len);
    CHECK('\0' == output[0]);
    CHECK("" == output);
}

//TEST_CASE("test_safe_snprintf_mocked") {
//    doctest::Context ctx;
//    char* argv[2] = {(char*)"c_unittests", nullptr};
////    char* argv[2] = {nullptr, nullptr};
//    spy::initialise(0, argv);
//    {
//        // add some spies about things that happen in f()
////        auto add_spy = SPY(&qd_port_int);
//        auto qdgsbn_spy = SPY(&qd_getservbyname);
//        auto fake = spy::fake(qdgsbn_spy, &fake2);
////        auto add_arg0 = spy::arg<0>(add_spy); // capture first argument of add()
////        auto add_arg1 = spy::arg<1>(add_spy); // capture second argument of add()
////        auto add_call = spy::call(add_spy);   // capture number of calls to add()
////        auto add_result = spy::result(add_spy); // capture return value from add()
//        auto rv = qd_port_int("amqps");
//        auto rvv = qd_port_int("amqp");
//                CHECK(rv == 5671);
//                CHECK(rvv == 5672);
////                CHECK(add_call.count() == 2);  // verify add is called twice
////                CHECK(add_arg0.value(0) == "amqps"); // verify first argument of first call
//////                CHECK(add_arg1.value(0) == 2); // verify second argument of first call
////                CHECK(add_arg0.value(1) == "amqp"); // verify first argument of second call
//////                CHECK(add_arg1.value(1) == 4); // verify second argument of second call
////                CHECK(add_result.value(0) == 5671); // verify return value of first call
////                CHECK(add_result.value(1) == 5672); // verify return value of second call
//    }
//}

TEST_CASE("test_safe_snprintf") {
    const int   OUTPUT_SIZE = 128;
    const char *TEST_MESSAGE = "something";
    const int   LEN = strlen(TEST_MESSAGE);

    size_t len;
    char   output[OUTPUT_SIZE];

    SUBCASE("valid_inputs") {
        SUBCASE("") {
            len = safe_snprintf(output, LEN + 10, TEST_MESSAGE);
            CHECK(LEN == len);
            CHECK(output == TEST_MESSAGE);
        }

        SUBCASE("") {
            len = safe_snprintf(output, LEN + 1, TEST_MESSAGE);
            CHECK(LEN == len);
            CHECK(output == TEST_MESSAGE);
        }

        SUBCASE("") {
            len = safe_snprintf(output, LEN, TEST_MESSAGE);
            CHECK(LEN - 1 == len);
            CHECK(output == "somethin");
        }

        SUBCASE("") {
            len = safe_snprintf(output, 0, TEST_MESSAGE);
            CHECK(0 == len);
        }

        SUBCASE("") {
            output[0] = 'a';
            len = safe_snprintf(output, 1, TEST_MESSAGE);
            CHECK(0 == len);
            CHECK('\0' == output[0]);
        }

        SUBCASE("") {
            len = safe_snprintf(output, (int)-1, TEST_MESSAGE);
            CHECK(0 == len);
        }
    }
}

TEST_CASE("test_qdr_terminus_format") {
    SUBCASE("coordinator") {
        const int   SIZE = 128;
        const char *EXPECTED = "{<coordinator>}";
        const int   EXPECTED_LEN = strlen(EXPECTED);

        size_t size = SIZE;
        char   output[SIZE];

        qdr_terminus_t t;
        t.coordinator = true;

        qdr_terminus_format(&t, output, &size);
        CHECK(output == EXPECTED);
        CHECK(size == SIZE - EXPECTED_LEN);
    }

    SUBCASE("empty") {
        char   output[3];
        size_t size = 3;
        output[2] = 'A';

        SUBCASE("") {
            qdr_terminus_format(NULL, output, &size);

            CHECK(output == "{}");
            CHECK(size == 1);
        }
    }
}
