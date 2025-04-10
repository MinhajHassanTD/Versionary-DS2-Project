#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

class Utils {
public:
    static bool fileExists(const std::string& filePath);
    static std::string readFile(const std::string& filePath);
    static void writeFile(const std::string& filePath, const std::string& content);
    static std::vector<std::string> splitString(const std::string& str, char delimiter);
};

#endif // UTILS_H