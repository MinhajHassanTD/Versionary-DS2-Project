#pragma once

#include "Version.h"
#include "MerkleTree.h"
#include "Quadtree.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <map>
#include <vector>

class VersionManager {
private:
    std::string repoPath;
    std::string currentBranch;
    std::string headVersionId;
    std::map<std::string, Version> versions;
    
    std::string getVersionsDir() const;
    std::string generateVersionId() const;
    std::string getTimestamp() const;
    
public:
    VersionManager();
    
    bool initRepository(const std::string& path);
    bool isRepository() const;
    
    bool saveImage(const cv::Mat& image, const std::string& versionId);
    cv::Mat loadImage(const std::string& versionId) const;
    
    bool saveVersionMetadata(const Version& version);
    bool loadVersionMetadata(const std::string& versionId, Version& version);
    bool loadAllVersions();
    
    bool addImage(const cv::Mat& image);
    std::string commit(const std::string& message, const cv::Mat& image);
    bool hasChanges(const cv::Mat& currentImage) const;
    
    std::vector<Version> getHistory() const;
    Version getVersion(const std::string& versionId) const;
    
    cv::Mat getDiff(const std::string& versionId1, const std::string& versionId2) const;
    cv::Mat getDiffWithCurrent(const cv::Mat& currentImage) const;
    
    std::string getRepoPath() const { return repoPath; }
    std::string getCurrentBranch() const { return currentBranch; }
    std::string getHeadVersionId() const { return headVersionId; }
};