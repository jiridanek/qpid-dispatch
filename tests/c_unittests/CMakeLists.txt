##
## Licensed to the Apache Software Foundation (ASF) under one
## or more contributor license agreements.  See the NOTICE file
## distributed with this work for additional information
## regarding copyright ownership.  The ASF licenses this file
## to you under the Apache License, Version 2.0 (the
## "License"); you may not use this file except in compliance
## with the License.  You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing,
## software distributed under the License is distributed on an
## "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
## KIND, either express or implied.  See the License for the
## specific language governing permissions and limitations
## under the License.
##

file(GLOB unittest_SOURCES
    "test_terminus.cpp"
    "c_unittests_main.cpp"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZE_FLAGS} -std=c++11")
add_executable(c_unittests ${unittest_SOURCES})
target_include_directories(c_unittests PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/)
target_link_libraries(c_unittests elfspy pthread qpid-dispatch)
target_compile_options(c_unittests PRIVATE -g -fPIE -fPIC -O0 -Wno-error -fpermissive -std=c++14 -Wl,-z,norelro)
target_link_options(c_unittests PRIVATE -g -fPIE -fPIC -O0 -Wno-error -fpermissive -std=c++14 -Wl,-z,norelro)

add_test(
    NAME c_unittests
    COMMAND ${TEST_WRAP} $<TARGET_FILE:c_unittests>
)

add_executable(mmck mmck.c)
target_include_directories(mmck PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/mimick/include)
target_link_libraries(mmck mimick pthread qpid-dispatch)
target_compile_options(mmck PRIVATE -g -fPIE -fPIC -O0 -Wno-error -fpermissive -std=c++14 -Wl,-z,norelro)
target_link_options(mmck PRIVATE -g -fPIE -fPIC -O0 -Wno-error -fpermissive -std=c++14 -Wl,-z,norelro)

add_test(
        NAME mmck
        COMMAND ${TEST_WRAP} $<TARGET_FILE:mmck>
)

#
#add_executable(c_unittestss test_terminuss.cpp "c_unittests_main.cpp")
#target_include_directories(c_unittestss PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tests/mimick/include)
#target_link_libraries(c_unittestss mimick pthread qpid-dispatch)
#target_compile_options(c_unittestss PRIVATE -g -fPIC -O0 -Wno-error -fpermissive -std=c++14)
#
#add_test(
#        NAME c_unittestss
#        COMMAND ${TEST_WRAP} $<TARGET_FILE:c_unittestss>
#)