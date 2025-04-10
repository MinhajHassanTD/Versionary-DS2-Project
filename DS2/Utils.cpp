#include "Utils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

// Checks if a file exists
bool Utils::fileExists(const std::string& filePath) {
    return std::filesystem::exists(filePath);
}

// Reads the content of a file
std::string Utils::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

// Writes content to a file
void Utils::writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    file << content;
}

// Splits a string by a delimiter
std::vector<std::string> Utils::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;

    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}
