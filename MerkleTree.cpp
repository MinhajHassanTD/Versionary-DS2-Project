#include "MerkleTree.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

// Constructor: Builds the Merkle Tree from data blocks
MerkleTree::MerkleTree(const std::vector<std::string>& dataBlocks) {
    tree = buildTree(dataBlocks);
}

// Returns the root hash of the Merkle Tree
std::string MerkleTree::getRootHash() const {
    return tree.empty() ? "" : tree.back();
}

// Builds the Merkle Tree and returns the tree structure
std::vector<std::string> MerkleTree::buildTree(const std::vector<std::string>& dataBlocks) {
    std::vector<std::string> currentLevel = dataBlocks;

    while (currentLevel.size() > 1) {
        std::vector<std::string> nextLevel;

        for (size_t i = 0; i < currentLevel.size(); i += 2) {
            if (i + 1 < currentLevel.size()) {
                nextLevel.push_back(hash(currentLevel[i] + currentLevel[i + 1]));
            } else {
                nextLevel.push_back(hash(currentLevel[i])); // Handle odd number of nodes
            }
        }

        currentLevel = nextLevel;
    }

    return currentLevel;
}

// Hashes a string using SHA-256
std::string MerkleTree::hash(const std::string& input) const {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    std::ostringstream oss;
    for (unsigned char c : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }

    return oss.str();
}
