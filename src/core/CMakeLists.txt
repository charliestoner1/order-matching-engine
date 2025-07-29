# Core library CMakeLists.txt

# Create core library
add_library(order_matching_core STATIC
    Order.cpp
    OrderBook.cpp
    MatchingEngine.cpp
    Trade.cpp
)

# Set include directories
target_include_directories(order_matching_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

# Set properties
set_target_properties(order_matching_core PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
