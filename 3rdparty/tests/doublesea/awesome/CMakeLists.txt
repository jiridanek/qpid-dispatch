project(awesome)

enable_testing()

add_library(shared_library SHARED lib1.h lib1.cpp)
add_library(static_library STATIC lib1.h lib1.cpp)

add_executable(test_elfspy_static test_elfspy.cpp)
target_link_libraries(test_elfspy_static doctest elfspy static_library)
add_test(test_elfspy_static EXECUTABLE test_elfspy_static)
target_compile_options(test_elfspy_static PRIVATE -fPIC -fPIE -Wl,--export-dynamic)  # not helping

add_executable(test_elfspy_dynamic test_elfspy.cpp)
target_link_libraries(test_elfspy_dynamic doctest elfspy shared_library)
add_test(test_elfspy_dynamic EXECUTABLE test_elfspy_dynamic)
target_compile_options(test_elfspy_dynamic PRIVATE -fPIC -no-pie)  # not helping
target_compile_options(test_elfspy_dynamic PRIVATE -no-pie)  # not helping

add_executable(test_mimick test_mimick.cpp)
target_link_libraries(test_mimick doctest mimick)
target_include_directories(test_mimick PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/mimick/include)
add_test(test_mimick EXECUTABLE test_mimick)
target_compile_options(test_mimick PRIVATE -fPIC -fPIE -Wl,--export-dynamic)

add_executable(test_plthook test_plthook.cpp)
target_link_libraries(test_plthook doctest plthook shared_library)
target_include_directories(test_plthook PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/plthook)
add_test(test_plthook EXECUTABLE test_plthook)
target_compile_options(test_plthook PRIVATE -fPIC -fPIE -Wl,--export-dynamic)

# LD_BIND_NOW=1 IPP_CONF_FILE=/home/jdanek/repos/qpid/qpid-dispatch/3rdparty/tests/isolator/usr/share/typemock/typemock.ini

# The function 'library_defined_function_cpp' information cannot be found by Isolator++:
#1) Compiler has not generate DEBUG information.
#2) Function is not used anywhere and has been optimized out by compiler.
#3) Wrong usage of FAKE instead of FAKE_GLOBAL(library_defined_function_cpp).
#To verify that DEBUG info is generated, please use -g3 option while compiling.
#Also make sure that optimization is switched off (-O0 compilation flag)
#**************************************
#For more info and help see:
#http://docs.typemock.com/isolatorpp

add_executable(test_isolator test_isolator.cpp)
target_link_libraries(test_isolator doctest isolator shared_library)
target_link_directories(test_isolator PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/isolator/usr/lib64/typemock)
target_include_directories(test_isolator PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/isolator/usr/include/typemock)
add_test(test_isolator EXECUTABLE test_isolator)
target_compile_options(test_isolator PRIVATE -fPIC -fno-PIE -Wl,--export-dynamic)
target_link_options(test_isolator PRIVATE -no-pie)

add_executable(test_self_call test_self_call.cpp)
#target_link_libraries(test_self_call shared_library)
#target_link_directories(test_self_call PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/isolator/usr/lib64/typemock)
#target_include_directories(test_self_call PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/isolator/usr/include/typemock)
#add_test(test_self_call EXECUTABLE test_self_call)
target_compile_options(test_self_call PRIVATE -fPIC -fno-PIE -Wl,--export-dynamic)
target_link_options(test_self_call PRIVATE -no-pie)