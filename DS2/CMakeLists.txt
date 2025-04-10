cmake_minimum_required(VERSION 3.10)
project(Versionary)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add source files
set(SOURCES
    main.cpp
    CLI.cpp
    GUI.cpp
    MerkleTree.cpp
    ImageProcessor.cpp
    Quadtree.cpp
    ImageComparer.cpp
    Utils.cpp
)

# Add executable for the main application
add_executable(Versionary ${SOURCES})

# Find and link OpenCV
find_package(OpenCV REQUIRED)
target_link_libraries(Versionary ${OpenCV_LIBS})

# Find and link OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(Versionary OpenSSL::SSL OpenSSL::Crypto)

# Comment out the Google Test section for now until you have test files
# You can uncomment this later when you have tests ready
#include(FetchContent)
#set(CMAKE_TLS_VERIFY OFF)
#FetchContent_Declare(
#  googletest
#  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
#  TLS_VERIFY OFF
#)
#FetchContent_MakeAvailable(googletest)
#
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#
# Add test executable
#add_executable(runTests
#    tests/MerkleTreeTest.cpp
#    # Add other test files here as needed
#)
#target_link_libraries(runTests gtest gtest_main ${OpenCV_LIBS} OpenSSL::SSL OpenSSL::Crypto)

# Add this at the end of your CMakeLists.txt
add_executable(TestConsole test.cpp)