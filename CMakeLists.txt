cmake_minimum_required(VERSION 3.10)
project(OrderMatchingEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(src)

# Main executable
add_executable(ordermatching
        src/main.cpp
        src/core/OrderBook.cpp
        src/implementations/BTreeOrderBook.cpp
)

# Test executable
add_executable(test_order_matching
        test/test_order_matching.cpp
        src/core/OrderBook.cpp
        src/implementations/BTreeOrderBook.cpp
)

# Benchmark executable (optional)
add_executable(benchmark
        benchmark/OrderBookBenchmark.cpp
        src/core/OrderBook.cpp
        src/implementations/BTreeOrderBook.cpp
)