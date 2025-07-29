# Source directory CMakeLists.txt

# Add subdirectories
add_subdirectory(core)
add_subdirectory(implementations)
add_subdirectory(utils)

# Create the main executable
add_executable(order_matching_engine main.cpp)

# Link libraries
target_link_libraries(order_matching_engine
    PRIVATE
        order_matching_core
        order_matching_implementations
        order_matching_utils
)

# Include directories
target_include_directories(order_matching_engine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
