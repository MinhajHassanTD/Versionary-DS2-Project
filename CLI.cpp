#include "CLI.h"
#include "Global.h" // Include the shared variables
#include "ImageProcessor.h"
#include "MerkleTree.h"
#include "Quadtree.h"
#include "ImageComparer.h"
#include "Utils.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <sstream> // Added for stringstream

// Runs the CLI loop
void CLI::run() {
    std::string command;
    while (true) {
        std::cout << "Versionary> ";
        std::getline(std::cin, command);

        std::cout << "Received command: " << command << std::endl; // Debug log

        if (command == "exit") {
            break;
        } else if (command.rfind("add ", 0) == 0) {
            handleAdd(command.substr(4));
        } else if (command == "commit") {
            handleCommit();
        } else if (command.rfind("compare ", 0) == 0) {
            size_t spacePos = command.find(' ', 8);
            if (spacePos != std::string::npos) {
                handleCompare(command.substr(8, spacePos - 8), command.substr(spacePos + 1));
            } else {
                std::cerr << "Invalid compare command. Use: compare <version1> <version2>\n";
            }
        } else if (command.rfind("view ", 0) == 0) {
            handleView(command.substr(5));
        } else if (command == "help") {
            printHelp();
        } else {
            std::cerr << "Unknown command. Type 'help' for a list of commands.\n";
        }
    }
}

// Handles the "add" command
void CLI::handleAdd(const std::string& filePath) {
    try {
        std::cout << "Reading image: " << filePath << "\n";
        cv::Mat image = ImageProcessor::readImage(filePath);

        std::cout << "Converting image to grayscale...\n";
        cv::Mat grayImage = ImageProcessor::convertToGrayscale(image);

        std::cout << "Chunking image using Quadtree...\n";
        if (grayImage.cols < 16 || grayImage.rows < 16) {
            throw std::runtime_error("Image dimensions are too small for Quadtree processing (minimum 16x16).");
        }

        Quadtree quadtree(grayImage, 16); // Minimum chunk size is 16x16

        std::cout << "Hashing image chunks using Merkle Tree...\n";
        std::vector<std::string> hashes;
        collectLeafHashes(quadtree.getRoot(), hashes);

        MerkleTree tree(hashes);
        std::string rootHash = tree.getRootHash();

        std::cout << "Image added successfully. Root hash: " << rootHash << "\n";

        versionRepository[++currentVersion] = rootHash;

        std::cout << "Current version repository size: " << versionRepository.size() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Helper function to collect hashes from leaf nodes
void CLI::collectLeafHashes(std::shared_ptr<QuadtreeNode> node, std::vector<std::string>& hashes) {
    if (!node) return;
    
    if (node->isLeaf()) {
        // Hash the image chunk data
        std::string hash = hashImageChunk(node->chunk);
        hashes.push_back(hash);
    } else {
        // Recursively collect hashes from children
        collectLeafHashes(node->topLeft, hashes);
        collectLeafHashes(node->topRight, hashes);
        collectLeafHashes(node->bottomLeft, hashes);
        collectLeafHashes(node->bottomRight, hashes);
    }
}

// Function to hash an image chunk
std::string CLI::hashImageChunk(const cv::Mat& chunk) {
    // Calculate mean pixel value as a simple hash
    cv::Scalar mean = cv::mean(chunk);
    std::stringstream ss;
    ss << "chunk_" << mean[0];
    return ss.str();
}

// Handles the "commit" command
void CLI::handleCommit() {
    try {
        if (versionRepository.empty()) {
            throw std::runtime_error("No images to commit.");
        }
        std::cout << "Committing version " << currentVersion << "...\n";
        std::cout << "Version " << currentVersion << " committed successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Handles the "compare" command
void CLI::handleCompare(const std::string& version1, const std::string& version2) {
    try {
        int v1 = std::stoi(version1);
        int v2 = std::stoi(version2);

        if (versionRepository.find(v1) == versionRepository.end() ||
            versionRepository.find(v2) == versionRepository.end()) {
            throw std::runtime_error("One or both versions do not exist.");
        }

        std::cout << "Comparing versions " << v1 << " and " << v2 << "...\n";
        
        // In a real implementation, we would load the actual images here
        // For now, we create two different dummy images to demonstrate comparison
        cv::Mat dummyImage1 = cv::Mat::zeros(100, 100, CV_8UC1);
        cv::Mat dummyImage2 = cv::Mat::ones(100, 100, CV_8UC1) * 255; // White image
        
        // Add some patterns to the images to make differences visible
        cv::circle(dummyImage1, cv::Point(50, 50), 20, cv::Scalar(255), -1);
        cv::rectangle(dummyImage2, cv::Rect(30, 30, 40, 40), cv::Scalar(0), -1);

        cv::Mat differences = ImageComparer::compareImages(dummyImage1, dummyImage2);
        ImageComparer::visualizeDifferences(differences, "differences_output.jpg");
        
        // No need to print this message here, it's already printed in visualizeDifferences
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Handles the "view" command
void CLI::handleView(const std::string& version) {
    try {
        int v = std::stoi(version);

        if (versionRepository.find(v) == versionRepository.end()) {
            throw std::runtime_error("Version does not exist.");
        }

        std::cout << "Viewing version " << v << "...\n";
        std::cout << "Root hash: " << versionRepository[v] << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Prints the help message
void CLI::printHelp() const {
    std::cout << "Available commands:\n";
    std::cout << "  add <file_path>       Add an image file to the repository.\n";
    std::cout << "  commit                Commit the current changes.\n";
    std::cout << "  compare <v1> <v2>     Compare two versions.\n";
    std::cout << "  view <version>        View a specific version.\n";
    std::cout << "  help                  Show this help message.\n";
    std::cout << "  exit                  Exit the application.\n";
}
