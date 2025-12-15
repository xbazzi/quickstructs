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

# Create header-only library interface
add_library(quickstructs INTERFACE)
target_include_directories(quickstructs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(quickstructs INTERFACE cxx_std_23)

# Compiler flags and other goodies
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)
find_package(Threads)
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

# Only build tests if this is the main project (not included via FetchContent)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  file(GLOB_RECURSE TEST_SOURCES
      "${CMAKE_SOURCE_DIR}/tests/*_test.cc"
      "${CMAKE_SOURCE_DIR}/tests/test.cc"
  )

  add_executable(quick_tests
      ${TEST_SOURCES}
  )

  set_target_properties(quick_tests PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED YES
  )

  target_include_directories(quick_tests PRIVATE
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/tests        # For test utilities and headers
      ${CMAKE_CURRENT_BINARY_DIR}      # For generated files
  )

  target_link_libraries(quick_tests PRIVATE
      GTest::gtest
      GTest::gtest_main
      Threads::Threads
  )

  # Register tests with CTest
  add_test(NAME quick_tests COMMAND quick_tests)
  
  # Optional: Use gtest_discover_tests for better test reporting
  gtest_discover_tests(quick_tests)
endif()

# ============================================================================
# Benchmarking Configuration
# ============================================================================

find_package(benchmark REQUIRED)

# Collect all benchmark source files
file(GLOB_RECURSE BENCHMARK_SOURCES
    "${CMAKE_SOURCE_DIR}/benchmarks/*_benchmark.cc"
)

if(BENCHMARK_SOURCES)
    add_executable(quick_benchmarks
        ${BENCHMARK_SOURCES}
    )

    set_target_properties(quick_benchmarks PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
    )

    target_include_directories(quick_benchmarks PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/benchmarks   # For benchmark utilities
        ${CMAKE_CURRENT_BINARY_DIR}      # For generated files
    )

    target_link_libraries(quick_benchmarks PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()

# ============================================================================
# Examples Configuration
# ============================================================================

# Collect all example source files
file(GLOB_RECURSE EXAMPLE_SOURCES
    "${CMAKE_SOURCE_DIR}/examples/*_example.cc"
)

# Build each example as a separate executable
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    
    add_executable(${EXAMPLE_NAME}
        ${EXAMPLE_SOURCE}
    )

    set_target_properties(${EXAMPLE_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
    )

    target_include_directories(${EXAMPLE_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    target_link_libraries(${EXAMPLE_NAME} PRIVATE
        Threads::Threads
    )
endforeach()
