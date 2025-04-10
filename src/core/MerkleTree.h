#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

class MerkleTree {
private:
    struct Node {
        std::string hash;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        
        Node(const std::string& h) : hash(h), left(nullptr), right(nullptr) {}
    };
    
    std::shared_ptr<Node> root;
    
    // Helper methods
    std::string hashData(const std::vector<uchar>& data);
    std::shared_ptr<Node> buildTree(const std::vector<std::string>& hashes);
    
public:
    MerkleTree() : root(nullptr) {}
    
    void buildFromImage(const cv::Mat& image);
    std::string getRootHash() const;
    bool compareWith(const MerkleTree& other) const;
};