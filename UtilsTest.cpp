#include "Utils.h"
#include <iostream>

int main() {
    std::string testFilePath = "test.txt";

    // Test fileExists
    if (Utils::fileExists(testFilePath)) {
        std::cout << "File exists: " << testFilePath << "\n";
    } else {
        std::cout << "File does not exist: " << testFilePath << "\n";
    }

    // Test writeFile and readFile
    Utils::writeFile(testFilePath, "Hello, Versionary!");
    std::string content = Utils::readFile(testFilePath);
    std::cout << "File content: " << content << "\n";

    // Test splitString
    std::vector<std::string> tokens = Utils::splitString("block1,block2,block3", ',');
    std::cout << "Split tokens:\n";
    for (const auto& token : tokens) {
        std::cout << token << "\n";
    }

    return 0;
}
//new
