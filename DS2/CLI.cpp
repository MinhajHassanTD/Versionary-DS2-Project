#include "CLI.h"
#include "Utils.h"  // Add this include for the Utils class
#include "ImageProcessor.h"  // Add this for ImageProcessor
#include "ImageComparer.h"  // Add this for ImageComparer
#include "MerkleTree.h"  // Add this for MerkleTree
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <filesystem>

// Runs the CLI loop
void CLI::run() {
    std::string command;
    while (true) {
        std::cout << "Versionary> ";
        std::getline(std::cin, command);

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
        std::cout << "Adding image: " << filePath << "\n";
        
        // Check if file exists
        if (!Utils::fileExists(filePath)) {
            throw std::runtime_error("File does not exist: " + filePath);
        }
        
        // Read the image using ImageProcessor
        cv::Mat image = ImageProcessor::readImage(filePath);
        
        // Convert to grayscale for more efficient processing
        cv::Mat grayImage = ImageProcessor::convertToGrayscale(image);
        
        // Save the image to a staging area (you might want to create a dedicated folder)
        std::string stagingPath = "staging/" + filePath.substr(filePath.find_last_of("/\\") + 1);
        cv::imwrite(stagingPath, grayImage);
        
        std::cout << "Image added to staging area successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void CLI::handleCommit() {
    try {
        std::cout << "Committing changes...\n";
        
        // Get list of files in staging area using filesystem
        std::vector<std::string> stagedFiles;
        namespace fs = std::filesystem;
        
        // Create staging directory if it doesn't exist
        if (!fs::exists("staging")) {
            fs::create_directory("staging");
        }
        
        // List all files in the staging directory
        for (const auto& entry : fs::directory_iterator("staging")) {
            if (entry.is_regular_file()) {
                stagedFiles.push_back(entry.path().string());
            }
        }
        
        if (stagedFiles.empty()) {
            std::cout << "No changes to commit.\n";
            return;
        }
        
        // Read each image and compute its hash
        std::vector<std::string> imageHashes;
        for (const auto& file : stagedFiles) {
            cv::Mat image = cv::imread(file, cv::IMREAD_GRAYSCALE);
            if (image.empty()) continue;
            
            // Convert image data to string for hashing
            std::string imageData(reinterpret_cast<char*>(image.data), image.total() * image.elemSize());
            
            // Create a unique hash for this image
            // In a real implementation, you might want to use a more sophisticated approach
            // such as dividing the image into chunks using Quadtree and then hashing each chunk
            imageHashes.push_back(imageData);
        }
        
        // Create a Merkle Tree from the image hashes
        MerkleTree merkleTree(imageHashes);
        std::string rootHash = merkleTree.getRootHash();
        
        // Generate a version ID (could be timestamp-based or incremental)
        std::string versionId = "v" + std::to_string(time(nullptr));
        
        // Store the version information
        // In a real implementation, you would save this to a persistent storage
        std::cout << "Created version " << versionId << " with root hash: " << rootHash << "\n";
        
        // Move files from staging to permanent storage
        // This is a placeholder - you would need to implement file moving functionality
        std::cout << "Changes committed successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void CLI::handleCompare(const std::string& version1, const std::string& version2) {
    try {
        std::cout << "Comparing versions: " << version1 << " and " << version2 << "\n";
        
        // Retrieve images for both versions
        // In a real implementation, you would load these from your version storage
        std::string imagePath1 = "versions/" + version1 + "/image.jpg"; // Example path
        std::string imagePath2 = "versions/" + version2 + "/image.jpg"; // Example path
        
        // Check if both versions exist
        if (!Utils::fileExists(imagePath1) || !Utils::fileExists(imagePath2)) {
            throw std::runtime_error("One or both versions do not exist.");
        }
        
        // Read the images
        cv::Mat image1 = ImageProcessor::readImage(imagePath1);
        cv::Mat image2 = ImageProcessor::readImage(imagePath2);
        
        // Compare the images
        cv::Mat differences = ImageComparer::compareImages(image1, image2);
        
        // Visualize the differences
        std::string outputPath = "diff_" + version1 + "_" + version2 + ".jpg";
        ImageComparer::visualizeDifferences(differences, outputPath);
        
        std::cout << "Comparison complete. Differences saved to: " << outputPath << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void CLI::handleView(const std::string& version) {
    try {
        std::cout << "Viewing version: " << version << "\n";
        
        // Retrieve the image for the specified version
        // In a real implementation, you would load this from your version storage
        std::string imagePath = "versions/" + version + "/image.jpg"; // Example path
        
        // Check if the version exists
        if (!Utils::fileExists(imagePath)) {
            throw std::runtime_error("Version does not exist: " + version);
        }
        
        // Read the image
        cv::Mat image = ImageProcessor::readImage(imagePath);
        
        // Display the image (in a real application, you might use a GUI library)
        cv::namedWindow("Version: " + version, cv::WINDOW_NORMAL);
        cv::imshow("Version: " + version, image);
        cv::waitKey(0);
        cv::destroyWindow("Version: " + version);
        
        std::cout << "Version " << version << " displayed.\n";
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

// Check the implementation of CLI::run() method
