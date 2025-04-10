#include "VersionaryCLI.h"
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

VersionaryCLI::VersionaryCLI() {
    currentDir = fs::current_path().string();
    
    // Initialize command map
    commandMap["init"] = [this](const std::vector<std::string>& args) { return handleInit(args); };
    commandMap["add"] = [this](const std::vector<std::string>& args) { return handleAdd(args); };
    commandMap["commit"] = [this](const std::vector<std::string>& args) { return handleCommit(args); };
    commandMap["status"] = [this](const std::vector<std::string>& args) { return handleStatus(args); };
    commandMap["log"] = [this](const std::vector<std::string>& args) { return handleLog(args); };
    commandMap["diff"] = [this](const std::vector<std::string>& args) { return handleDiff(args); };
}

void VersionaryCLI::setCurrentDirectory(const std::string& dir) {
    currentDir = dir;
}

std::vector<std::string> VersionaryCLI::parseArgs(const std::string& commandLine) {
    std::vector<std::string> args;
    std::istringstream iss(commandLine);
    std::string arg;
    
    bool inQuotes = false;
    std::string quotedArg;
    
    while (iss >> arg) {
        if (!inQuotes && arg.front() == '"' && arg.back() != '"') {
            // Start of quoted argument
            inQuotes = true;
            quotedArg = arg.substr(1);
        } else if (inQuotes && arg.back() == '"') {
            // End of quoted argument
            quotedArg += " " + arg.substr(0, arg.length() - 1);
            args.push_back(quotedArg);
            inQuotes = false;
        } else if (inQuotes) {
            // Middle of quoted argument
            quotedArg += " " + arg;
        } else {
            // Regular argument
            args.push_back(arg);
        }
    }
    
    return args;
}

bool VersionaryCLI::executeCommand(const std::string& commandLine) {
    std::vector<std::string> args = parseArgs(commandLine);
    
    if (args.empty()) {
        return false;
    }
    
    std::string command = args[0];
    args.erase(args.begin());
    
    auto it = commandMap.find(command);
    if (it != commandMap.end()) {
        return it->second(args);
    } else if (command == "help") {
        printHelp();
        return true;
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Type 'help' for usage information." << std::endl;
        return false;
    }
}

void VersionaryCLI::printHelp() {
    std::cout << "Versionary - Image Version Control System" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  init                  Initialize a new repository" << std::endl;
    std::cout << "  add <image>           Add an image to staging" << std::endl;
    std::cout << "  commit -m \"message\"   Create a new version" << std::endl;
    std::cout << "  status                Show repository status" << std::endl;
    std::cout << "  log                   Show version history" << std::endl;
    std::cout << "  diff [id1] [id2]      Show differences between versions" << std::endl;
    std::cout << "  help                  Show this help message" << std::endl;
}

bool VersionaryCLI::handleInit(const std::vector<std::string>& args) {
    if (versionManager.initRepository(currentDir)) {
        std::cout << "Initialized empty Versionary repository in " << currentDir << std::endl;
        return true;
    } else {
        std::cout << "Failed to initialize repository" << std::endl;
        return false;
    }
}

bool VersionaryCLI::handleAdd(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: No image specified" << std::endl;
        return false;
    }
    
    std::string imagePath = args[0];
    cv::Mat image = cv::imread(imagePath);
    
    if (image.empty()) {
        std::cout << "Error: Could not read image " << imagePath << std::endl;
        return false;
    }
    
    if (versionManager.addImage(image)) {
        std::cout << "Added image " << imagePath << " to staging" << std::endl;
        // For demo, save as staged_image.png
        cv::imwrite(currentDir + "/staged_image.png", image);
        return true;
    } else {
        std::cout << "Failed to add image" << std::endl;
        return false;
    }
}

bool VersionaryCLI::handleCommit(const std::vector<std::string>& args) {
    std::string message;
    
    // Parse -m flag
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "-m" && i + 1 < args.size()) {
            message = args[i + 1];
            break;
        }
    }
    
    if (message.empty()) {
        std::cout << "Error: Commit message required (-m \"message\")" << std::endl;
        return false;
    }
    
    // For demo, read from staged_image.png
    cv::Mat stagedImage = cv::imread(currentDir + "/staged_image.png");
    
    if (stagedImage.empty()) {
        std::cout << "Error: No image staged for commit" << std::endl;
        return false;
    }
    
    std::string versionId = versionManager.commit(message, stagedImage);
    
    if (!versionId.empty()) {
        std::cout << "Created version " << versionId << std::endl;
        // Remove staged image after commit
        fs::remove(currentDir + "/staged_image.png");
        return true;
    } else {
        std::cout << "Failed to create version" << std::endl;
        return false;
    }
}

bool VersionaryCLI::handleStatus(const std::vector<std::string>& args) {
    if (!versionManager.isRepository()) {
        std::cout << "Not a Versionary repository" << std::endl;
        return false;
    }
    
    std::cout << "On branch " << versionManager.getCurrentBranch() << std::endl;
    
    // For demo, check if staged_image.png exists
    if (fs::exists(currentDir + "/staged_image.png")) {
        std::cout << "Changes to be committed:" << std::endl;
        std::cout << "  (use \"versionary reset\" to unstage)" << std::endl;
        std::cout << "        new file: staged_image.png" << std::endl;
    } else {
        std::cout << "No changes staged for commit" << std::endl;
    }
    
    return true;
}

bool VersionaryCLI::handleLog(const std::vector<std::string>& args) {
    if (!versionManager.isRepository()) {
        std::cout << "Not a Versionary repository" << std::endl;
        return false;
    }
    
    std::vector<Version> history = versionManager.getHistory();
    
    if (history.empty()) {
        std::cout << "No commits yet" << std::endl;
        return true;
    }
    
    for (const auto& version : history) {
        std::cout << "Version: " << version.id << std::endl;
        std::cout << "Date: " << version.timestamp << std::endl;
        std::cout << "Message: " << version.message << std::endl;
        std::cout << std::endl;
    }
    
    return true;
}

bool VersionaryCLI::handleDiff(const std::vector<std::string>& args) {
    if (!versionManager.isRepository()) {
        std::cout << "Not a Versionary repository" << std::endl;
        return false;
    }
    
    cv::Mat diffImage;
    
    if (args.empty()) {
        // Diff between staged and HEAD
        cv::Mat stagedImage = cv::imread(currentDir + "/staged_image.png");
        
        if (stagedImage.empty()) {
            std::cout << "No staged image to diff" << std::endl;
            return false;
        }
        
        diffImage = versionManager.getDiffWithCurrent(stagedImage);
    } else if (args.size() == 1) {
        // Diff between specified version and HEAD
        std::string versionId = args[0];
        std::string headId = versionManager.getHeadVersionId();
        
        if (headId.empty()) {
            std::cout << "No HEAD version to diff against" << std::endl;
            return false;
        }
        
        diffImage = versionManager.getDiff(versionId, headId);
    } else if (args.size() >= 2) {
        // Diff between two specified versions
        std::string versionId1 = args[0];
        std::string versionId2 = args[1];
        
        diffImage = versionManager.getDiff(versionId1, versionId2);
    }
    
    if (diffImage.empty()) {
        std::cout << "Failed to generate diff" << std::endl;
        return false;
    }
    
    // Display diff image
    cv::imshow("Diff", diffImage);
    cv::waitKey(0);
    
    return true;
}