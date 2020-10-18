cmake_minimum_required(VERSION 2.8.12...3.16)
project(seaspy)
#enable_language(CPP)

add_library(seaspy src/spy.cpp)

enable_testing()

add_library(doctest OBJECT ${CMAKE_SOURCE_DIR}/tests/c_unittests/c_unittests_main.cpp)
target_include_directories(doctest INTERFACE ${CMAKE_SOURCE_DIR}/tests/c_unittests)

add_executable(seaspy_test src/test_spy.cpp)
target_link_libraries(seaspy PRIVATE plthook)

target_link_libraries(seaspy_test PRIVATE doctest seaspy plthook)
target_include_directories(seaspy_test PRIVATE ${CMAKE_SOURCE_DIR}/tests/c_unittests)
target_include_directories(seaspy_test PRIVATE ../plthook)
target_link_options(seaspy_test PRIVATE -fPIC -no-pie)
target_compile_options(seaspy_test PRIVATE -fPIC -no-pie)
add_test(seaspy_test COMMAND seaspy_test)
