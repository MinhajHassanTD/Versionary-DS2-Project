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

# Set OpenCV_DIR to help CMake find OpenCV
set(OpenCV_DIR "C:/opencv/build/x64/vc16/lib")

# Find OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    target_link_libraries(Versionary ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV or check the path.")
endif()

# Set OpenSSL paths - using DLL files instead of lib files
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "C:/Program Files/OpenSSL-Win64/include")
set(OPENSSL_CRYPTO_LIBRARY "C:/Program Files/OpenSSL-Win64/bin/libcrypto-3-x64.dll")
set(OPENSSL_SSL_LIBRARY "C:/Program Files/OpenSSL-Win64/bin/libssl-3-x64.dll")

# Find and link OpenSSL
# Make OpenSSL optional
find_package(OpenSSL QUIET)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
    target_link_libraries(Versionary OpenSSL::SSL OpenSSL::Crypto)
    add_definitions(-DOPENSSL_AVAILABLE)
else()
    message(WARNING "OpenSSL not found. Building without OpenSSL support.")
    add_definitions(-DOPENSSL_NOT_AVAILABLE)
endif()

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
# Only link against OpenSSL if it was found
if(OPENSSL_FOUND)
    target_link_libraries(TestConsole ${OpenCV_LIBS} OpenSSL::SSL OpenSSL::Crypto)
else()
    target_link_libraries(TestConsole ${OpenCV_LIBS})
endif()