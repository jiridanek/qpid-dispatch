

add_subdirectory(tests/mimick)

file(GLOB elfspy_SOURCES
    "tests/elfspy/*.cpp"
)

add_library(elfspy ${elfspy_SOURCES})
target_include_directories(elfspy PRIVATE tests)