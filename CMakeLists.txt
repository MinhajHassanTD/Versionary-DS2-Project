cmake_minimum_required(VERSION 3.10)
project(Versionary VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Set source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Qt specific setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create a static library with the core functionality
add_library(versionary_lib STATIC
    src/MerkleTree.cpp
    src/Quadtree.cpp
    src/ImageProcessor.cpp
    src/VersionControl.cpp
)

# Link the library with required dependencies
target_link_libraries(versionary_lib ${OpenCV_LIBS})

# Add executable
add_executable(versionary ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(versionary versionary_lib ${OpenCV_LIBS} Qt5::Widgets)

# Create directory structure if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# Installation
install(TARGETS versionary DESTINATION bin)

# Add tests
add_subdirectory(tests)
