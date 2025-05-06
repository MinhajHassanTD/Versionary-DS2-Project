#ifndef MERKLETREE_H
#define MERKLETREE_H

#include <string>
#include <vector>

class MerkleTree {
public:
    MerkleTree(const std::vector<std::string>& dataBlocks);
    std::string getRootHash() const;

private:
    std::vector<std::string> buildTree(const std::vector<std::string>& dataBlocks);
    std::string hash(const std::string& input) const;

    std::vector<std::string> tree;
};

#endif // MERKLETREE_H