cmake_minimum_required(VERSION 3.14)
project(OrderMatchingEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Common flags
    add_compile_options(-Wall -Wextra -Wpedantic)

    # Release flags for maximum performance
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

    # Debug flags
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

    # Profile flags
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
endif()

# MSVC flags
if(MSVC)
    add_compile_options(/W4)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(external)

# Optional components

option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(BUILD_VISUALIZATION "Build visualization server" ON)



if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(BUILD_VISUALIZATION)
    add_subdirectory(visualization/server)
endif()

# Print configuration
message(STATUS "")
message(STATUS "Order Matching Engine Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "  Build visualization: ${BUILD_VISUALIZATION}")
message(STATUS "")