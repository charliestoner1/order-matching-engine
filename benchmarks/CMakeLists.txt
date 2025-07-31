add_executable(order_book_benchmark
    OrderBookBenchmark.cpp
)

target_link_libraries(order_book_benchmark
    PRIVATE
        order_matching_core
        order_matching_implementations
        order_matching_utils
)

target_include_directories(order_book_benchmark
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)
