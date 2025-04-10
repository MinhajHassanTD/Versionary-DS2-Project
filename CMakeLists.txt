cmake_minimum_required(VERSION 3.10)
project(Versionary VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Add source files
set(SOURCES
    src/main.cpp
    src/core/MerkleTree.cpp
    src/core/Quadtree.cpp
    src/core/VersionManager.cpp
    src/cli/CommandParser.cpp
    src/gui/MainWindow.cpp
)

# Add header files
set(HEADERS
    src/core/MerkleTree.h
    src/core/Quadtree.h
    src/core/VersionManager.h
    src/cli/CommandParser.h
    src/gui/MainWindow.h
)

# Set automoc for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create executable
add_executable(versionary ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(versionary 
    ${OpenCV_LIBS}
    ${OPENSSL_LIBRARIES}
    Qt5::Widgets
)

# Install target
install(TARGETS versionary DESTINATION bin)