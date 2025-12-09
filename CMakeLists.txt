cmake_minimum_required(VERSION 4.0)
project(QuickStructs CXX)
enable_testing()
include(CTest)
include(GoogleTest)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message("*** Build type not set.  defaulting to Debug")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options( 
        -g3 
        -Og
        -std=c++23
        -fsanitize=address,undefined,leak
        -fno-omit-frame-pointer 
        -Wall
        -Wconversion
        -Wpedantic
        -Werror
        -Wextra
        -Wno-interference-size
    )
else()
    add_compile_options(
        -O3
        -std=c++23
        -Wall
        -Wconversion
        -Wpedantic
        -Werror
        -Wextra
        -march=native
        -Wall
    )
endif()
add_link_options(-Wno-interference-size -fsanitize=address,undefined,leak)

# add quickstructs library from src/quickstructs/
# add_library(quickstructs
#     src/quickstructs/
# )

# Compiler flags and other goodies
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
include(FetchContent)

include_directories("${CMAKE_SOURCE_DIR}/include")

# Put all executables in the same directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Convention for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message("************************************************************")
message("**** Building in ${CMAKE_BUILD_TYPE} mode ")
message("************************************************************")

# ============================================================================
# Testing Configuration
# ============================================================================

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_SOURCE_DIR}/tests/*_test.cc"
    "${CMAKE_SOURCE_DIR}/tests/test.cc"
)

add_executable(unit_tests
    ${TEST_SOURCES}
)

set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests        # For test utilities and headers
    ${CMAKE_CURRENT_BINARY_DIR}      # For generated files
)

target_link_libraries(unit_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
)

# Register tests with CTest
add_test(NAME unit_tests COMMAND unit_tests)

# ============================================================================
# Benchmarking Configuration
# ============================================================================

find_package(benchmark REQUIRED)

# Collect all benchmark source files
file(GLOB_RECURSE BENCHMARK_SOURCES
    "${CMAKE_SOURCE_DIR}/benchmarks/*_benchmark.cc"
)

if(BENCHMARK_SOURCES)
    add_executable(benchmarks
        ${BENCHMARK_SOURCES}
    )

    set_target_properties(benchmarks PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
    )

    target_include_directories(benchmarks PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/benchmarks   # For benchmark utilities
        ${CMAKE_CURRENT_BINARY_DIR}      # For generated files
    )

    target_link_libraries(benchmarks PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()

# Optional: Use gtest_discover_tests for better test reporting
gtest_discover_tests(unit_tests)
