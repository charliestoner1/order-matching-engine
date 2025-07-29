add_library(order_matching_utils STATIC
    Timer.cpp
    ThreadPool.cpp
)

target_include_directories(order_matching_utils
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)
