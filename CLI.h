#ifndef CLI_H
#define CLI_H

#include <string>
#include <vector>
#include "Quadtree.h"

class CLI {
public:
    void run();
private:
    void handleAdd(const std::string& filePath);
    void handleCommit();
    void handleCompare(const std::string& version1, const std::string& version2, int sensitivity = 65);
    void handleAdvancedCompare(const std::string& version1, const std::string& version2, int chunkSize = 16, int sensitivity = 10);
    void handleView(const std::string& version);
    void handleDelete(const std::string& version); 
    void handleList();
    void printHelp() const;
    
    // Helper functions for hashing
    void collectLeafHashes(std::shared_ptr<QuadtreeNode> node, std::vector<std::string>& hashes);
    std::string hashImageChunk(const cv::Mat& chunk);
};

#endif // CLI_H
