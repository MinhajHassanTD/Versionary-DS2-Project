#include "MerkleTree.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

// Helper function to compute SHA-256 hash
std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.length());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

MerkleTree::MerkleTree(const std::vector<std::string>& leafHashes) {
    // Create leaf nodes
    leaves_.reserve(leafHashes.size());
    for (const auto& hash : leafHashes) {
        leaves_.push_back(std::make_shared<Node>(hash));
    }
    
    // Build the tree
    if (!leaves_.empty()) {
        root_ = buildTree(leaves_);
    }
}

std::string MerkleTree::getRootHash() const {
    return root_ ? root_->hash : "";
}

std::shared_ptr<MerkleTree::Node> MerkleTree::getRoot() const {
    return root_;
}

std::vector<size_t> MerkleTree::findDifferences(const MerkleTree& other) const {
    std::vector<size_t> diffIndices;
    
    // If root hashes are the same, no differences
    if (getRootHash() == other.getRootHash()) {
        return diffIndices;
    }
    
    // Find differences recursively
    findDifferencesRecursive(root_, other.getRoot(), diffIndices, 0, leaves_.size() - 1);
    
    return diffIndices;
}

std::vector<std::string> MerkleTree::getProof(size_t leafIndex) const {
    std::vector<std::string> proof;
    
    if (leafIndex >= leaves_.size()) {
        return proof;
    }
    
    size_t currentIndex = leafIndex;
    size_t levelSize = leaves_.size();
    std::vector<std::shared_ptr<Node>> currentLevel = leaves_;
    
    while (levelSize > 1) {
        // Determine if current node is left or right child
        bool isLeftChild = (currentIndex % 2 == 0);
        size_t siblingIndex = isLeftChild ? currentIndex + 1 : currentIndex - 1;
        
        // Add sibling hash to proof if it exists
        if (siblingIndex < levelSize) {
            proof.push_back(currentLevel[siblingIndex]->hash);
        }
        
        // Move to parent level
        std::vector<std::shared_ptr<Node>> nextLevel;
        for (size_t i = 0; i < levelSize; i += 2) {
            if (i + 1 < levelSize) {
                std::string combinedHash = combineHashes(currentLevel[i]->hash, currentLevel[i + 1]->hash);
                nextLevel.push_back(std::make_shared<Node>(combinedHash));
            } else {
                nextLevel.push_back(currentLevel[i]);
            }
        }
        
        currentIndex /= 2;
        currentLevel = nextLevel;
        levelSize = currentLevel.size();
    }
    
    return proof;
}

bool MerkleTree::verifyProof(const std::string& leafHash, 
                           const std::vector<std::string>& proof, 
                           const std::string& rootHash, 
                           size_t leafIndex) {
    std::string computedHash = leafHash;
    size_t currentIndex = leafIndex;
    
    for (const auto& proofElement : proof) {
        if (currentIndex % 2 == 0) {
            // Current node is left child
            computedHash = combineHashes(computedHash, proofElement);
        } else {
            // Current node is right child
            computedHash = combineHashes(proofElement, computedHash);
        }
        currentIndex /= 2;
    }
    
    return computedHash == rootHash;
}

std::shared_ptr<MerkleTree::Node> MerkleTree::buildTree(const std::vector<std::shared_ptr<Node>>& leaves) {
    if (leaves.empty()) {
        return nullptr;
    }
    
    if (leaves.size() == 1) {
        return leaves[0];
    }
    
    std::vector<std::shared_ptr<Node>> parents;
    
    for (size_t i = 0; i < leaves.size(); i += 2) {
        if (i + 1 < leaves.size()) {
            // Create parent node with combined hash
            std::string combinedHash = combineHashes(leaves[i]->hash, leaves[i + 1]->hash);
            auto parent = std::make_shared<Node>(combinedHash);
            parent->left = leaves[i];
            parent->right = leaves[i + 1];
            parents.push_back(parent);
        } else {
            // Odd number of nodes, promote the last one
            parents.push_back(leaves[i]);
        }
    }
    
    // Recursively build the tree
    return buildTree(parents);
}

void MerkleTree::findDifferencesRecursive(const std::shared_ptr<Node>& node1, 
                                        const std::shared_ptr<Node>& node2, 
                                        std::vector<size_t>& indices, 
                                        size_t start, 
                                        size_t end) const {
    // If hashes are the same, no differences in this subtree
    if (node1->hash == node2->hash) {
        return;
    }
    
    // If leaf node, add index to differences
    if (start == end) {
        indices.push_back(start);
        return;
    }
    
    // Calculate middle index
    size_t mid = start + (end - start) / 2;
    
    // Recursively check left subtree if it exists
    if (node1->left && node2->left) {
        findDifferencesRecursive(node1->left, node2->left, indices, start, mid);
    }
    
    // Recursively check right subtree if it exists
    if (node1->right && node2->right && mid + 1 <= end) {
        findDifferencesRecursive(node1->right, node2->right, indices, mid + 1, end);
    }
}

std::string MerkleTree::combineHashes(const std::string& left, const std::string& right) {
    return sha256(left + right);
}
