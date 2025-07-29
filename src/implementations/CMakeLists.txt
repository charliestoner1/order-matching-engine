add_library(order_matching_implementations STATIC
    RBTreeOrderBook.cpp
    BTreeOrderBook.cpp
)

target_include_directories(order_matching_implementations
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(order_matching_implementations
    PUBLIC
        order_matching_core
)
