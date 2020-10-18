project(awesome)

enable_testing()

add_executable(test_elfspy test_elfspy.cpp)
target_link_libraries(test_elfspy doctest elfspy)
add_test(test_elfspy EXECUTABLE test_elfspy)
target_compile_options(test_elfspy PRIVATE -fPIC -fPIE)  # not helping

add_executable(test_mimick test_mimick.cpp)
target_link_libraries(test_mimick doctest mimick)
target_include_directories(test_mimick PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/mimick/include)
add_test(test_mimick EXECUTABLE test_mimick)
target_compile_options(test_mimick PRIVATE -fPIC -fPIE)