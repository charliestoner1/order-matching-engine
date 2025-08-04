cmake_minimum_required(VERSION 3.10)
project(OrderMatchingEngine VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags for maximum performance
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops -fomit-frame-pointer -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Add additional warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(benchmark)

# Core library
add_library(order_matching_core STATIC
        src/core/OrderBook.cpp
)

# B-Tree implementation
add_library(btree_orderbook STATIC
        src/implementations/BTreeOrderBook.cpp
)

# Main executable
add_executable(ordermatching
        src/main.cpp
)

# Benchmark executable
add_executable(benchmark
        benchmark/OrderBookBenchmark.cpp
)

# Link libraries
target_link_libraries(ordermatching
        order_matching_core
        btree_orderbook
)

target_link_libraries(benchmark
        order_matching_core
        btree_orderbook
)

# Enable Link Time Optimization for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ordermatching PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Install targets
install(TARGETS ordermatching DESTINATION bin)
install(TARGETS benchmark DESTINATION bin)