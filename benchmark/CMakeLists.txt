add_executable(order_book_benchmark
        OrderBookBenchmark.cpp
)

# Force the correct link order using LINK_LIBRARIES property
set_target_properties(order_book_benchmark PROPERTIES
        LINK_LIBRARIES "btree_orderbook;order_matching_core"
)

target_include_directories(order_book_benchmark
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)