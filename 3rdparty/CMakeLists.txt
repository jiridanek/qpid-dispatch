

add_subdirectory(tests/mimick)

file(GLOB elfspy_SOURCES
    "tests/elfspy/*.cpp"
)

add_library(elfspy ${elfspy_SOURCES})
target_include_directories(elfspy PRIVATE tests)

add_library(glibcmock tests/glibcmock/got_hook.cc tests/glibcmock/got_hook.h)
target_include_directories(glibcmock PRIVATE tests/glibcmock)
target_include_directories(glibcmock INTERFACE tests/glibcmock/test)
target_link_options(glibcmock PRIVATE)
#target_link_options(glibcmock PRIVATE -no-pie)

file(GLOB glibmock_test_SOURCES
        "tests/glibcmock/test/*.cc"
        )

add_executable(glibmock_tests ${glibmock_test_SOURCES})
target_include_directories(glibmock_tests PRIVATE tests/glibcmock)
target_link_libraries(glibmock_tests glibcmock -lpthread)
#target_link_options(glibmock_tests PRIVATE)
target_link_options(glibmock_tests PRIVATE -no-pie)
add_test(NAME glibmock_tests COMMAND glibmock_tests --gtest_death_test_style=threadsafe)

add_library(plthook tests/plthook/plthook_elf.c)
target_link_libraries(plthook -ldl)
target_include_directories(plthook PRIVATE tests/plthook)

#add_executable(plthook_tests ${plthook_test_SOURCES})
#target_include_directories(glibmock_tests PRIVATE tests/glibcmock)
#target_link_libraries(glibmock_tests glibcmock -lpthread)
##target_link_options(glibmock_tests PRIVATE)
#target_link_options(glibmock_tests PRIVATE -no-pie)
#add_test(NAME glibmock_tests COMMAND glibmock_tests --gtest_death_test_style=threadsafe)

add_subdirectory(tests/seaspy)
