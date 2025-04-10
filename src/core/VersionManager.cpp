#include "VersionManager.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

VersionManager::VersionManager() : currentBranch("main"), headVersionId("") {}

bool VersionManager::initRepository(const std::string& path) {
    repoPath = path;
    
    // Create repository directory structure
    try {
        if (!fs::exists(repoPath)) {
            fs::create_directories(repoPath);
        }
        
        std::string versionaryDir = repoPath + "/.versionary";
        if (!fs::exists(versionaryDir)) {
            fs::create_directories(versionaryDir);
            fs::create_directories(versionaryDir + "/versions");
            fs::create_directories(versionaryDir + "/branches");
            
            // Initialize main branch
            std::ofstream branchFile(versionaryDir + "/branches/main");
            branchFile << ""; // Empty head commit for now
            branchFile.close();
            
            // Create config file
            std::ofstream configFile(versionaryDir + "/config.json");
            json config;
            config["current_branch"] = "main";
            configFile << config.dump(4);
            configFile.close();
            
            currentBranch = "main";
            headVersionId = "";
            return true;
        }
        
        // If repository already exists, load its state
        return loadAllVersions();
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing repository: " << e.what() << std::endl;
        return false;
    }
}

bool VersionManager::isRepository() const {
    return fs::exists(repoPath + "/.versionary");
}

std::string VersionManager::generateVersionId() const {
    // Generate a random SHA-1 like hash
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex_chars = "0123456789abcdef";
    std::string uuid;
    
    for (int i = 0; i < 40; i++) {
        uuid += hex_chars[dis(gen)];
    }
    
    return uuid;
}

std::string VersionManager::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool VersionManager::saveImage(const cv::Mat& image, const std::string& versionId) {
    std::string versionsDir = getVersionsDir();
    std::string imagePath = versionsDir + "/" + versionId + ".png";
    
    return cv::imwrite(imagePath, image);
}

cv::Mat VersionManager::loadImage(const std::string& versionId) const {
    std::string versionsDir = getVersionsDir();
    std::string imagePath = versionsDir + "/" + versionId + ".png";
    
    if (!fs::exists(imagePath)) {
        return cv::Mat();
    }
    
    return cv::imread(imagePath);
}

bool VersionManager::saveVersionMetadata(const Version& version) {
    std::string versionsDir = getVersionsDir();
    std::string metadataPath = versionsDir + "/" + version.id + ".json";
    
    try {
        json metadata;
        metadata["id"] = version.id;
        metadata["message"] = version.message;
        metadata["timestamp"] = version.timestamp;
        metadata["parent_id"] = version.parentId;
        metadata["image_path"] = version.imagePath;
        metadata["merkle_root_hash"] = version.merkleRootHash;
        
        std::ofstream file(metadataPath);
        file << metadata.dump(4);
        file.close();
        
        // Update branch head
        std::string branchPath = repoPath + "/.versionary/branches/" + currentBranch;
        std::ofstream branchFile(branchPath);
        branchFile << version.id;
        branchFile.close();
        
        // Update head version ID
        headVersionId = version.id;
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving version metadata: " << e.what() << std::endl;
        return false;
    }
}

bool VersionManager::loadVersionMetadata(const std::string& versionId, Version& version) {
    std::string versionsDir = getVersionsDir();
    std::string metadataPath = versionsDir + "/" + versionId + ".json";
    
    if (!fs::exists(metadataPath)) {
        return false;
    }
    
    try {
        std::ifstream file(metadataPath);
        json metadata;
        file >> metadata;
        
        version.id = metadata["id"];
        version.message = metadata["message"];
        version.timestamp = metadata["timestamp"];
        version.parentId = metadata["parent_id"];
        version.imagePath = metadata["image_path"];
        version.merkleRootHash = metadata["merkle_root_hash"];
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading version metadata: " << e.what() << std::endl;
        return false;
    }
}

bool VersionManager::loadAllVersions() {
    std::string versionsDir = getVersionsDir();
    
    if (!fs::exists(versionsDir)) {
        return false;
    }
    
    versions.clear();
    
    try {
        // Load all version metadata files
        for (const auto& entry : fs::directory_iterator(versionsDir)) {
            if (entry.path().extension() == ".json") {
                std::string versionId = entry.path().stem().string();
                Version version;
                
                if (loadVersionMetadata(versionId, version)) {
                    versions[versionId] = version;
                }
            }
        }
        
        // Load current branch and head
        std::string configPath = repoPath + "/.versionary/config.json";
        if (fs::exists(configPath)) {
            std::ifstream configFile(configPath);
            json config;
            configFile >> config;
            currentBranch = config["current_branch"];
        }
        
        // Load head commit ID
        std::string branchPath = repoPath + "/.versionary/branches/" + currentBranch;
        if (fs::exists(branchPath)) {
            std::ifstream branchFile(branchPath);
            std::getline(branchFile, headVersionId);
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading versions: " << e.what() << std::endl;
        return false;
    }
}

std::string VersionManager::getVersionsDir() const {
    return repoPath + "/.versionary/versions";
}

bool VersionManager::addImage(const cv::Mat& image) {
    // For now, we'll just store the image in memory
    // In a real implementation, we might want to store it in a staging area
    return !image.empty();
}

std::string VersionManager::commit(const std::string& message, const cv::Mat& image) {
    if (image.empty()) {
        std::cerr << "No image provided for commit" << std::endl;
        return "";
    }
    
    // Generate a new version ID
    std::string versionId = generateVersionId();
    
    // Calculate Merkle tree hash
    MerkleTree merkleTree;
    merkleTree.buildFromImage(image);
    std::string merkleRootHash = merkleTree.getRootHash();
    
    // Save the image
    if (!saveImage(image, versionId)) {
        std::cerr << "Failed to save image for version " << versionId << std::endl;
        return "";
    }
    
    // Create version metadata
    Version version;
    version.id = versionId;
    version.message = message;
    version.timestamp = getTimestamp();
    version.parentId = headVersionId;
    version.imagePath = versionId + ".png";
    version.merkleRootHash = merkleRootHash;
    
    // Save metadata
    if (!saveVersionMetadata(version)) {
        std::cerr << "Failed to save metadata for version " << versionId << std::endl;
        return "";
    }
    
    // Add to versions map
    versions[versionId] = version;
    
    return versionId;
}

bool VersionManager::hasChanges(const cv::Mat& currentImage) const {
    if (headVersionId.empty()) {
        return !currentImage.empty(); // If no commits yet, any image is a change
    }
    
    cv::Mat headImage = loadImage(headVersionId);
    if (headImage.empty()) {
        return true;
    }
    
    // Use Merkle tree to quickly check if images are identical
    MerkleTree currentTree, headTree;
    currentTree.buildFromImage(currentImage);
    headTree.buildFromImage(headImage);
    
    return !currentTree.compareWith(headTree);
}

std::vector<Version> VersionManager::getHistory() const {
    std::vector<Version> history;
    
    // Start from head and follow parent links
    std::string currentId = headVersionId;
    while (!currentId.empty()) {
        auto it = versions.find(currentId);
        if (it != versions.end()) {
            history.push_back(it->second);
            currentId = it->second.parentId;
        } else {
            break;
        }
    }
    
    return history;
}

Version VersionManager::getVersion(const std::string& versionId) const {
    auto it = versions.find(versionId);
    if (it != versions.end()) {
        return it->second;
    }
    
    return Version(); // Return empty version if not found
}

cv::Mat VersionManager::getDiff(const std::string& versionId1, const std::string& versionId2) const {
    cv::Mat image1 = loadImage(versionId1);
    cv::Mat image2 = loadImage(versionId2);
    
    if (image1.empty() || image2.empty()) {
        return cv::Mat(); // Return empty mat if either image can't be loaded
    }
    
    // Use Quadtree to find differences
    Quadtree tree1, tree2;
    tree1.buildFromImage(image1);
    tree2.buildFromImage(image2);
    
    std::vector<cv::Rect> differences = tree1.compareWith(tree2);
    
    // Visualize differences
    return tree1.visualizeDiff(image1, differences);
}

cv::Mat VersionManager::getDiffWithCurrent(const cv::Mat& currentImage) const {
    if (headVersionId.empty()) {
        return cv::Mat(); // No head version to compare with
    }
    
    cv::Mat headImage = loadImage(headVersionId);
    
    if (headImage.empty()) {
        return cv::Mat();
    }
    
    // Use Quadtree to find differences
    Quadtree currentTree, headTree;
    currentTree.buildFromImage(currentImage);
    headTree.buildFromImage(headImage);
    
    std::vector<cv::Rect> differences = currentTree.compareWith(headTree);
    
    // Visualize differences
    return currentTree.visualizeDiff(currentImage, differences);
}