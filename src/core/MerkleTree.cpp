#include "MerkleTree.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

void MerkleTree::buildFromImage(const cv::Mat& image) {
    // Divide image into blocks and hash each block
    const int blockSize = 16;
    std::vector<std::string> blockHashes;
    
    for (int y = 0; y < image.rows; y += blockSize) {
        for (int x = 0; x < image.cols; x += blockSize) {
            // Extract block
            cv::Rect blockRect(x, y, 
                std::min(blockSize, image.cols - x), 
                std::min(blockSize, image.rows - y));
            cv::Mat block = image(blockRect);
            
            // Convert block to byte array
            std::vector<uchar> blockData;
            if (block.isContinuous()) {
                blockData.assign(block.data, block.data + block.total() * block.channels());
            } else {
                for (int i = 0; i < block.rows; ++i) {
                    blockData.insert(blockData.end(), block.ptr<uchar>(i), 
                                    block.ptr<uchar>(i) + block.cols * block.channels());
                }
            }
            
            // Hash the block
            std::string blockHash = hashData(blockData);
            blockHashes.push_back(blockHash);
        }
    }
    
    // Build the tree from block hashes
    root = buildTree(blockHashes);
}

std::string MerkleTree::getRootHash() const {
    if (root) {
        return root->hash;
    }
    return "";
}

bool MerkleTree::compareWith(const MerkleTree& other) const {
    // Quick comparison of root hashes
    if (!root || !other.root) {
        return false;
    }
    
    return root->hash == other.root->hash;
}

std::string MerkleTree::hashData(const std::vector<uchar>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

std::shared_ptr<MerkleTree::Node> MerkleTree::buildTree(const std::vector<std::string>& hashes) {
    if (hashes.empty()) {
        return nullptr;
    }
    
    // Create leaf nodes
    std::vector<std::shared_ptr<Node>> nodes;
    for (const auto& hash : hashes) {
        nodes.push_back(std::make_shared<Node>(hash));
    }
    
    // Build tree bottom-up
    while (nodes.size() > 1) {
        std::vector<std::shared_ptr<Node>> parents;
        
        for (size_t i = 0; i < nodes.size(); i += 2) {
            if (i + 1 < nodes.size()) {
                // Create parent with two children
                std::string combinedHash = nodes[i]->hash + nodes[i+1]->hash;
                std::vector<uchar> combinedData(combinedHash.begin(), combinedHash.end());
                std::string parentHash = hashData(combinedData);
                
                auto parent = std::make_shared<Node>(parentHash);
                parent->left = nodes[i];
                parent->right = nodes[i+1];
                parents.push_back(parent);
            } else {
                // Odd number of nodes, promote the last one
                parents.push_back(nodes[i]);
            }
        }
        
        nodes = parents;
    }
    
    return nodes.front();
}