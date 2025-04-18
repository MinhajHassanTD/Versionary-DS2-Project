#include "GUI.h"
#include "Global.h" // Include the shared variables
#include "ImageProcessor.h"
#include "MerkleTree.h"
#include "Quadtree.h"
#include "ImageComparer.h"
#include "Utils.h"
#include <iostream>

// Initializes the GUI
void GUI::initialize() {
    std::cout << "Initializing GUI...\n";
    displayMainMenu();
}

// Displays the main menu
void GUI::displayMainMenu() {
    while (true) {
        std::cout << "\nVersionary - Main Menu\n";
        std::cout << "1. Add Image\n";
        std::cout << "2. Compare Images\n";
        std::cout << "3. View Version\n";
        std::cout << "4. Exit\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1:
                handleAddImage();
                break;
            case 2:
                handleCompareImages();
                break;
            case 3:
                handleViewVersion();
                break;
            case 4:
                handleExit();
                return;
            default:
                showError("Invalid choice. Please try again.");
        }
    }
}

// Handles adding an image
void GUI::handleAddImage() {
    std::string filePath;
    std::cout << "Enter the file path of the image to add: ";
    std::cin >> filePath;

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
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

// Handles comparing images
void GUI::handleCompareImages() {
    std::string version1, version2;
    std::cout << "Enter the first version to compare: ";
    std::cin >> version1;
    std::cout << "Enter the second version to compare: ";
    std::cin >> version2;

    try {
        int v1 = std::stoi(version1);
        int v2 = std::stoi(version2);

        if (versionRepository.find(v1) == versionRepository.end() ||
            versionRepository.find(v2) == versionRepository.end()) {
            throw std::runtime_error("One or both versions do not exist.");
        }

        std::cout << "Comparing versions " << v1 << " and " << v2 << "...\n";
        // Simulate loading images and comparing
        cv::Mat dummyImage1 = cv::Mat::zeros(100, 100, CV_8UC1);
        cv::Mat dummyImage2 = cv::Mat::zeros(100, 100, CV_8UC1);

        cv::Mat differences = ImageComparer::compareImages(dummyImage1, dummyImage2);
        ImageComparer::visualizeDifferences(differences, "differences_output.jpg");

        std::cout << "Differences visualized and saved to differences_output.jpg\n";
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

// Handles viewing a specific version
void GUI::handleViewVersion() {
    std::string version;
    std::cout << "Enter the version to view: ";
    std::cin >> version;

    try {
        int v = std::stoi(version);

        if (versionRepository.find(v) == versionRepository.end()) {
            throw std::runtime_error("Version does not exist.");
        }

        std::cout << "Viewing version " << v << "...\n";
        std::cout << "Root hash: " << versionRepository[v] << "\n";
        // Simulate loading and displaying the image
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

// Handles exiting the application
void GUI::handleExit() {
    std::cout << "Exiting Versionary. Goodbye!\n";
}

// Displays an error message
void GUI::showError(const std::string& message) {
    std::cerr << "Error: " << message << "\n";
}

// Helper function to collect hashes from leaf nodes
void GUI::collectLeafHashes(std::shared_ptr<QuadtreeNode> node, std::vector<std::string>& hashes) {
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
std::string GUI::hashImageChunk(const cv::Mat& chunk) {
    // Calculate mean pixel value as a simple hash
    cv::Scalar mean = cv::mean(chunk);
    std::stringstream ss;
    ss << "chunk_" << mean[0];
    return ss.str();
}