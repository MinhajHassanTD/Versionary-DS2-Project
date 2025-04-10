#pragma once

#include "../core/VersionManager.h"
#include <string>
#include <vector>
#include <functional>
#include <map>

class VersionaryCLI {
private:
    VersionManager versionManager;
    std::string currentDir;
    
    // Command handlers
    bool handleInit(const std::vector<std::string>& args);
    bool handleAdd(const std::vector<std::string>& args);
    bool handleCommit(const std::vector<std::string>& args);
    bool handleStatus(const std::vector<std::string>& args);
    bool handleLog(const std::vector<std::string>& args);
    bool handleDiff(const std::vector<std::string>& args);
    
    // Command map
    std::map<std::string, std::function<bool(const std::vector<std::string>&)>> commandMap;
    
    // Helper methods
    std::vector<std::string> parseArgs(const std::string& commandLine);
    
public:
    VersionaryCLI();
    
    void setCurrentDirectory(const std::string& dir);
    bool executeCommand(const std::string& commandLine);
    void printHelp();
    
    std::string getCurrentDirectory() const;
};