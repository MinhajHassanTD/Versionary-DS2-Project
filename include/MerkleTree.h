#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <vector>
#include <string>
#include <memory>
#include <functional>

/**
 * @brief MerkleTree class for efficient version tracking
 * 
 * This class implements a Merkle Tree data structure for tracking
 * image versions. It uses SHA-256 hashing to create unique identifiers
 * for image chunks and efficiently detect changes between versions.
 */
class MerkleTree {
public:
    /**
     * @brief Node structure for the Merkle Tree
     */
    struct Node {
        std::string hash;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        
        Node(const std::string& hash) : hash(hash), left(nullptr), right(nullptr) {}
    };

    /**
     * @brief Construct a new Merkle Tree from a list of leaf hashes
     * 
     * @param leafHashes Vector of hash strings representing leaf nodes
     */
    MerkleTree(const std::vector<std::string>& leafHashes);
    
    /**
     * @brief Get the root hash of the Merkle Tree
     * 
     * @return std::string Root hash
     */
    std::string getRootHash() const;
    
    /**
     * @brief Get the root node of the Merkle Tree
     * 
     * @return std::shared_ptr<Node> Root node
     */
    std::shared_ptr<Node> getRoot() const;
    
    /**
     * @brief Compare this Merkle Tree with another to find differences
     * 
     * @param other Another Merkle Tree to compare with
     * @return std::vector<size_t> Indices of different leaf nodes
     */
    std::vector<size_t> findDifferences(const MerkleTree& other) const;
    
    /**
     * @brief Get the proof path for a specific leaf
     * 
     * @param leafIndex Index of the leaf node
     * @return std::vector<std::string> Proof path hashes
     */
    std::vector<std::string> getProof(size_t leafIndex) const;
    
    /**
     * @brief Verify a proof for a specific leaf
     * 
     * @param leafHash Hash of the leaf node
     * @param proof Proof path hashes
     * @param rootHash Root hash to verify against
     * @param leafIndex Index of the leaf node
     * @return bool True if the proof is valid
     */
    static bool verifyProof(const std::string& leafHash, 
                           const std::vector<std::string>& proof, 
                           const std::string& rootHash, 
                           size_t leafIndex);

private:
    std::shared_ptr<Node> root_;
    std::vector<std::shared_ptr<Node>> leaves_;
    
    /**
     * @brief Build the Merkle Tree from leaf nodes
     * 
     * @param leaves Vector of leaf nodes
     * @return std::shared_ptr<Node> Root node of the built tree
     */
    std::shared_ptr<Node> buildTree(const std::vector<std::shared_ptr<Node>>& leaves);
    
    /**
     * @brief Find differences between two trees recursively
     * 
     * @param node1 Node from first tree
     * @param node2 Node from second tree
     * @param indices Vector to store indices of different leaves
     * @param start Start index in the leaf array
     * @param end End index in the leaf array
     */
    void findDifferencesRecursive(const std::shared_ptr<Node>& node1, 
                                 const std::shared_ptr<Node>& node2, 
                                 std::vector<size_t>& indices, 
                                 size_t start, 
                                 size_t end) const;
    
    /**
     * @brief Combine two hashes to create a parent hash
     * 
     * @param left Left child hash
     * @param right Right child hash
     * @return std::string Combined hash
     */
    static std::string combineHashes(const std::string& left, const std::string& right);
};

#endif // MERKLE_TREE_H
