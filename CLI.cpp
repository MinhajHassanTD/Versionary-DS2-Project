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

        // Store the version information
        versionRepository[++currentVersion] = rootHash;
        
        // Save the image for future reference
        cv::imwrite("version_" + std::to_string(currentVersion) + ".jpg", image);
        std::cout << "Image saved as version_" + std::to_string(currentVersion) + ".jpg\n";

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
        // First, check if the inputs are valid integers
        for (char c : version1) {
            if (!std::isdigit(c)) {
                throw std::invalid_argument("Version numbers must be integers");
            }
        }
        
        for (char c : version2) {
            if (!std::isdigit(c)) {
                throw std::invalid_argument("Version numbers must be integers");
            }
        }
        
        // Now safely convert to integers
        int v1 = std::stoi(version1);
        int v2 = std::stoi(version2);

        if (versionRepository.find(v1) == versionRepository.end() ||
            versionRepository.find(v2) == versionRepository.end()) {
            throw std::runtime_error("One or both versions do not exist.");
        }

        std::cout << "Comparing versions " << v1 << " and " << v2 << "...\n";
        
        // Get the filenames of the stored images
        std::string imagePath1 = "version_" + std::to_string(v1) + ".jpg";
        std::string imagePath2 = "version_" + std::to_string(v2) + ".jpg";
        
        // Load the saved images
        cv::Mat image1 = cv::imread(imagePath1);
        cv::Mat image2 = cv::imread(imagePath2);
        
        // Check if the images were loaded successfully
        if (image1.empty() || image2.empty()) {
            // For demonstration purposes, create dummy images if the saved ones can't be loaded
            std::cout << "Warning: Could not load saved images. Using dummy images for demonstration.\n";
            
            image1 = cv::Mat::zeros(300, 300, CV_8UC3);
            image2 = image1.clone();
            
            // Add some content to the first image
            cv::circle(image1, cv::Point(150, 150), 100, cv::Scalar(255, 0, 0), -1);
            cv::putText(image1, "Version " + std::to_string(v1), cv::Point(80, 280), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            
            // Add slightly different content to the second image
            cv::circle(image2, cv::Point(150, 150), 80, cv::Scalar(0, 0, 255), -1);
            cv::rectangle(image2, cv::Rect(50, 50, 80, 60), cv::Scalar(0, 255, 0), -1);
            cv::putText(image2, "Version " + std::to_string(v2), cv::Point(80, 280), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        }
        
        // Save copies of the images we're comparing
        cv::imwrite("original_" + std::to_string(v1) + ".jpg", image1);
        cv::imwrite("original_" + std::to_string(v2) + ".jpg", image2);
        
        std::cout << "Original images saved as original_" << std::to_string(v1) << ".jpg and original_" 
                  << std::to_string(v2) << ".jpg\n";
        
        // Compare images and visualize differences directly on the original image
        cv::Mat differences = ImageComparer::compareImages(image1, image2);
        ImageComparer::visualizeDifferences(differences, "differences_output.jpg");
        
        std::cout << "Differences have been highlighted on the first image and saved to differences_output.jpg\n";
        
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Version number out of range\n";
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
