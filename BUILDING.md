# Building Versionary

This document provides instructions for building and running the Versionary image-based version control system.

## Prerequisites

Before building Versionary, you need to install the following dependencies:

### OpenCV

Versionary uses OpenCV for image processing. You can install it as follows:

#### Windows

1. Download the OpenCV installer from the [official website](https://opencv.org/releases/).
2. Run the installer and follow the instructions.
3. Add the OpenCV bin directory to your PATH environment variable.

#### Linux

```bash
sudo apt-get update
sudo apt-get install libopencv-dev
```

#### macOS

```bash
brew install opencv
```

### Qt

Versionary uses Qt for the graphical user interface. You can install it as follows:

#### Windows

1. Download the Qt installer from the [official website](https://www.qt.io/download).
2. Run the installer and follow the instructions.
3. Add the Qt bin directory to your PATH environment variable.

#### Linux

```bash
sudo apt-get update
sudo apt-get install qt5-default qtcreator
```

#### macOS

```bash
brew install qt
```

### CMake

Versionary uses CMake as its build system. You can install it as follows:

#### Windows

1. Download the CMake installer from the [official website](https://cmake.org/download/).
2. Run the installer and follow the instructions.
3. Add the CMake bin directory to your PATH environment variable.

#### Linux

```bash
sudo apt-get update
sudo apt-get install cmake
```

#### macOS

```bash
brew install cmake
```

### OpenSSL

Versionary uses OpenSSL for hashing. You can install it as follows:

#### Windows

1. Download the OpenSSL installer from the [official website](https://slproweb.com/products/Win32OpenSSL.html).
2. Run the installer and follow the instructions.
3. Add the OpenSSL bin directory to your PATH environment variable.

#### Linux

```bash
sudo apt-get update
sudo apt-get install libssl-dev
```

#### macOS

```bash
brew install openssl
```

### nlohmann/json

Versionary uses nlohmann/json for JSON parsing. You can install it as follows:

#### Windows

```bash
vcpkg install nlohmann-json
```

#### Linux

```bash
sudo apt-get update
sudo apt-get install nlohmann-json3-dev
```

#### macOS

```bash
brew install nlohmann-json
```

## Building

Once you have installed all the dependencies, you can build Versionary as follows:

1. Clone the repository:

```bash
git clone https://github.com/yourusername/versionary.git
cd versionary
```

2. Create a build directory:

```bash
mkdir build
cd build
```

3. Configure the build:

```bash
cmake ..
```

4. Build the project:

```bash
cmake --build .
```

## Running

After building the project, you can run Versionary in either GUI or CLI mode:

### GUI Mode

```bash
./versionary --gui
```

### CLI Mode

```bash
./versionary --cli [command] [args]
```

For example:

```bash
./versionary --cli init
./versionary --cli add image.png
./versionary --cli commit "Initial commit"
./versionary --cli list
```

## Running Tests

To run the tests, use the following commands:

```bash
cd build
./tests/test_merkle_tree
./tests/test_quadtree
```

## Documentation

For more information on how to use Versionary, please refer to the [User Guide](USER_GUIDE.md).
