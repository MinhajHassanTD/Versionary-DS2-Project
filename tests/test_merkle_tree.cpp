#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "MerkleTree.h"

void testMerkleTreeConstruction() {
    std::vector<std::string> leafHashes = {
        "hash1", "hash2", "hash3", "hash4"
    };
    
    MerkleTree tree(leafHashes);
    
    // Check that root hash is not empty
    assert(!tree.getRootHash().empty());
    
    std::cout << "MerkleTree construction test passed!" << std::endl;
}

void testMerkleTreeComparison() {
    std::vector<std::string> leafHashes1 = {
        "hash1", "hash2", "hash3", "hash4"
    };
    
    std::vector<std::string> leafHashes2 = {
        "hash1", "hash2", "hash3", "hash5" // Different last hash
    };
    
    MerkleTree tree1(leafHashes1);
    MerkleTree tree2(leafHashes2);
    
    // Trees should be different
    assert(tree1.getRootHash() != tree2.getRootHash());
    
    // Find differences
    std::vector<size_t> differences = tree1.findDifferences(tree2);
    
    // Should find one difference at index 3
    assert(differences.size() == 1);
    assert(differences[0] == 3);
    
    std::cout << "MerkleTree comparison test passed!" << std::endl;
}

void testMerkleTreeProof() {
    std::vector<std::string> leafHashes = {
        "hash1", "hash2", "hash3", "hash4"
    };
    
    MerkleTree tree(leafHashes);
    
    // Get proof for leaf 2
    std::vector<std::string> proof = tree.getProof(2);
    
    // Verify proof
    bool valid = MerkleTree::verifyProof(leafHashes[2], proof, tree.getRootHash(), 2);
    assert(valid);
    
    std::cout << "MerkleTree proof test passed!" << std::endl;
}

int main() {
    std::cout << "Running MerkleTree tests..." << std::endl;
    
    testMerkleTreeConstruction();
    testMerkleTreeComparison();
    testMerkleTreeProof();
    
    std::cout << "All MerkleTree tests passed!" << std::endl;
    
    return 0;
}
