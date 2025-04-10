#pragma once

#include <string>

struct Version {
    std::string id;
    std::string message;
    std::string timestamp;
    std::string parentId;
    std::string imagePath;
    std::string merkleRootHash;
    
    Version() = default;
};