#ifndef CLI_H
#define CLI_H

#include <string>

class CLI {
public:
    void run();
private:
    void handleAdd(const std::string& filePath);
    void handleCommit();
    void handleCompare(const std::string& version1, const std::string& version2);
    void handleView(const std::string& version);
    void printHelp() const;
};

#endif // CLI_H
