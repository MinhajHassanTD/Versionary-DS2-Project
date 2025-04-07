cmake_minimum_required(VERSION 3.10)

# Add test executables
add_executable(test_merkle_tree test_merkle_tree.cpp)
add_executable(test_quadtree test_quadtree.cpp)

# Link libraries
target_link_libraries(test_merkle_tree ${OpenCV_LIBS})
target_link_libraries(test_quadtree ${OpenCV_LIBS})

# Add include directories
target_include_directories(test_merkle_tree PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(test_quadtree PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link with source files
target_link_libraries(test_merkle_tree 
    ${CMAKE_BINARY_DIR}/libversionary.a
)

target_link_libraries(test_quadtree 
    ${CMAKE_BINARY_DIR}/libversionary.a
)
