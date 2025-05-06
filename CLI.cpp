#include "CLI.h"
#include "Global.h"
#include "ImageProcessor.h"
#include "MerkleTree.h"
#include "Quadtree.h"
#include "ImageComparer.h"
#include "Utils.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <sstream>
#include <chrono>

// Main CLI command loop
void CLI::run() {
    // Load the version repository
    loadVersionRepository();
    
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
            std::istringstream iss(command.substr(8));
            std::string v1, v2;
            int sensitivity = 65; // Default sensitivity
            
            if (!(iss >> v1 >> v2)) {
                std::cerr << "Invalid compare command. Use: compare <version1> <version2> [sensitivity]\n";
                continue;
            }
            
            // Optional sensitivity parameter
            iss >> sensitivity;
            
            handleCompare(v1, v2, sensitivity);
        } else if (command.rfind("advcompare ", 0) == 0) {
            std::istringstream iss(command.substr(11));
            std::string v1, v2;
            int chunkSize = 16; // Default chunk size
            int sensitivity = 10; // Default sensitivity
            
            if (!(iss >> v1 >> v2)) {
                std::cerr << "Invalid advcompare command. Use: advcompare <version1> <version2> [chunkSize] [sensitivity]\n";
                continue;
            }
            
            // Optional parameters
            iss >> chunkSize;
            if (chunkSize < 8) chunkSize = 8; // Minimum reasonable chunk size
            
            iss >> sensitivity;
            
            handleAdvancedCompare(v1, v2, chunkSize, sensitivity);
        } else if (command.rfind("view ", 0) == 0) {
            handleView(command.substr(5));
        } else if (command.rfind("delete ", 0) == 0) {
            handleDelete(command.substr(7));
        } else if (command == "list") {
            handleList();
        } else if (command == "help") {
            printHelp();
        } else {
            std::cerr << "Unknown command. Type 'help' for a list of commands.\n";
        }
    }
}

// Adds a new image to the repository
void CLI::handleAdd(const std::string& filePath) {
    try {
        std::cout << "Processing image...\n";
        cv::Mat image = ImageProcessor::readImage(filePath);
        cv::Mat grayImage = ImageProcessor::convertToGrayscale(image);

        if (grayImage.cols < 16 || grayImage.rows < 16) {
            throw std::runtime_error("Image dimensions are too small for Quadtree processing (minimum 16x16).");
        }

        Quadtree quadtree(grayImage, 16); // Minimum chunk size is 16x16

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

        // Save the version repository
        saveVersionRepository();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Collects hashes from all leaf nodes in the Quadtree
void CLI::collectLeafHashes(std::shared_ptr<QuadtreeNode> node, std::vector<std::string>& hashes) {
    if (!node) return;
    
    if (node->isLeaf()) {
        std::string hash = hashImageChunk(node->chunk);
        hashes.push_back(hash);
    } else {
        collectLeafHashes(node->topLeft, hashes);
        collectLeafHashes(node->topRight, hashes);
        collectLeafHashes(node->bottomLeft, hashes);
        collectLeafHashes(node->bottomRight, hashes);
    }
}

// Computes a perceptual hash for an image chunk
std::string CLI::hashImageChunk(const cv::Mat& chunk) {
    return Utils::computePerceptualHash(chunk);
}

// Commits the current version
void CLI::handleCommit() {
    try {
        if (versionRepository.empty()) {
            throw std::runtime_error("No images to commit.");
        }
        std::cout << "Version " << currentVersion << " committed successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Compares two versions using pixel-by-pixel approach
void CLI::handleCompare(const std::string& version1, const std::string& version2, int sensitivity) {
    try {
        // Validate version numbers
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
        
        int v1 = std::stoi(version1);
        int v2 = std::stoi(version2);

        if (versionRepository.find(v1) == versionRepository.end() ||
            versionRepository.find(v2) == versionRepository.end()) {
            throw std::runtime_error("One or both versions do not exist.");
        }

        std::cout << "Comparing versions " << v1 << " and " << v2 << "...\n";
        
        // Load saved images
        std::string imagePath1 = "version_" + std::to_string(v1) + ".jpg";
        std::string imagePath2 = "version_" + std::to_string(v2) + ".jpg";
        cv::Mat image1 = cv::imread(imagePath1);
        cv::Mat image2 = cv::imread(imagePath2);
        
        // Create dummy images if needed for demonstration
        if (image1.empty() || image2.empty()) {
            std::cout << "Warning: Could not load saved images. Using dummy images for demonstration.\n";
            
            image1 = cv::Mat::zeros(300, 300, CV_8UC3);
            image2 = image1.clone();
            
            // Add content to the images
            cv::circle(image1, cv::Point(150, 150), 100, cv::Scalar(255, 0, 0), -1);
            cv::putText(image1, "Version " + std::to_string(v1), cv::Point(80, 280), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            
            cv::circle(image2, cv::Point(150, 150), 80, cv::Scalar(0, 0, 255), -1);
            cv::rectangle(image2, cv::Rect(50, 50, 80, 60), cv::Scalar(0, 255, 0), -1);
            cv::putText(image2, "Version " + std::to_string(v2), cv::Point(80, 280), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        }
        
        // Compare images using specified sensitivity
        cv::Mat differences = ImageComparer::compareImages(image1, image2, sensitivity);
        ImageComparer::visualizeDifferences(differences, "differences_output.jpg");
        
        std::cout << "Comparing with sensitivity threshold: " << sensitivity 
                  << " (higher = less sensitive)" << std::endl;
        std::cout << "Differences have been highlighted and saved to differences_output.jpg\n";
        
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Version number out of range\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Deletes a specific version from the repository
void CLI::handleDelete(const std::string& version) {
    try {
        // Validate version number
        for (char c : version) {
            if (!std::isdigit(c)) {
                throw std::invalid_argument("Version number must be an integer");
            }
        }
        
        int v = std::stoi(version);

        if (versionRepository.find(v) == versionRepository.end()) {
            throw std::runtime_error("Version " + version + " does not exist.");
        }

        // Don't allow deleting the latest version
        if (v == currentVersion) {
            throw std::runtime_error("Cannot delete the current version. Please commit a new version first.");
        }

        // Remove the version from the repository
        versionRepository.erase(v);
        
        // Delete the image file
        std::string imagePath = "version_" + version + ".jpg";
        if (remove(imagePath.c_str()) != 0) {
            std::cout << "Warning: Could not delete the image file: " << imagePath << std::endl;
        }

        std::cout << "Version " << v << " has been deleted successfully.\n";
        saveVersionRepository();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Version number out of range\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Lists all versions in the repository
void CLI::handleList() {
    try {
        if (versionRepository.empty()) {
            std::cout << "No versions found in the repository.\n";
            return;
        }

        std::cout << "Versions in the repository:\n";
        std::cout << "-------------------------\n";
        std::cout << "Current version: " << currentVersion << "\n";
        std::cout << "-------------------------\n";
        std::cout << "Version | Root Hash\n";
        std::cout << "-------------------------\n";
        
        for (const auto& pair : versionRepository) {
            std::string marker = (pair.first == currentVersion) ? " (current)" : "";
            std::cout << pair.first << marker << " | " << pair.second.substr(0, 16) << "...\n";
        }
        std::cout << "-------------------------\n";
        std::cout << "Total versions: " << versionRepository.size() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Displays information about a specific version
void CLI::handleView(const std::string& version) {
    try {
        // Validate version number
        for (char c : version) {
            if (!std::isdigit(c)) {
                throw std::invalid_argument("Version number must be an integer");
            }
        }
        
        int v = std::stoi(version);

        if (versionRepository.find(v) == versionRepository.end()) {
            throw std::runtime_error("Version " + version + " does not exist.");
        }

        std::cout << "Viewing version " << v << "...\n";
        std::cout << "Root hash: " << versionRepository[v] << "\n";
        
        // Load and display the image
        std::string imagePath = "version_" + version + ".jpg";
        cv::Mat image = cv::imread(imagePath);
        
        if (image.empty()) {
            std::cout << "Warning: Could not load image file for version " << v << std::endl;
            return;
        }
        
        // Display image information
        std::cout << "Image dimensions: " << image.cols << " x " << image.rows << std::endl;
        std::cout << "Image channels: " << image.channels() << std::endl;
        
        // Show image in a window
        std::string windowName = "Version " + version;
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::imshow(windowName, image);
        
        std::cout << "Image displayed. Press any key to continue...\n";
        cv::waitKey(0);
        cv::destroyWindow(windowName);
        
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Version number out of range\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

// Compares two versions using the advanced Merkle/Quadtree approach
void CLI::handleAdvancedCompare(const std::string& version1, const std::string& version2, int chunkSize, int sensitivity) {
    try {
        // Validate version numbers
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
        
        int v1 = std::stoi(version1);
        int v2 = std::stoi(version2);

        if (versionRepository.find(v1) == versionRepository.end() ||
            versionRepository.find(v2) == versionRepository.end()) {
            throw std::runtime_error("One or both versions do not exist.");
        }

        std::cout << "Advanced comparison in progress..." << std::endl;
        
        // Load saved images
        std::string imagePath1 = "version_" + std::to_string(v1) + ".jpg";
        std::string imagePath2 = "version_" + std::to_string(v2) + ".jpg";
        cv::Mat image1 = cv::imread(imagePath1);
        cv::Mat image2 = cv::imread(imagePath2);
        
        // Create dummy images if needed for demonstration
        if (image1.empty() || image2.empty()) {
            std::cout << "Warning: Could not load saved images. Using dummy images for demonstration.\n";
            
            image1 = cv::Mat::zeros(300, 300, CV_8UC3);
            image2 = image1.clone();
            
            cv::circle(image1, cv::Point(150, 150), 100, cv::Scalar(255, 0, 0), -1);
            cv::putText(image1, "Version " + std::to_string(v1), cv::Point(80, 280), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            
            cv::circle(image2, cv::Point(150, 150), 80, cv::Scalar(0, 0, 255), -1);
            cv::rectangle(image2, cv::Rect(50, 50, 80, 60), cv::Scalar(0, 255, 0), -1);
            cv::putText(image2, "Version " + std::to_string(v2), cv::Point(80, 280), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        }
        
        // Time the advanced comparison
        auto startTime = std::chrono::high_resolution_clock::now();
        std::vector<cv::Rect> diffRegions = ImageComparer::compareWithStructures(image1, image2, chunkSize, sensitivity);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Prepare result image with highlighted differences
        cv::Mat resultImage = image1.clone();
        
        for (const auto& region : diffRegions) {
            // Ensure region is within image bounds
            cv::Rect safeRegion = region & cv::Rect(0, 0, resultImage.cols, resultImage.rows);
            if (safeRegion.width <= 0 || safeRegion.height <= 0) continue;
            
            // Create a semi-transparent red overlay
            cv::Mat overlay = resultImage(safeRegion).clone();
            cv::addWeighted(overlay, 0.5, cv::Scalar(0, 0, 255), 0.5, 0, overlay);
            overlay.copyTo(resultImage(safeRegion));
            
            // Draw a green border
            cv::rectangle(resultImage, safeRegion, cv::Scalar(0, 255, 0), 2);
        }
        
        // Save the result
        cv::imwrite("adv_differences_output.jpg", resultImage);
        
        std::cout << "Advanced comparison with chunk size: " << chunkSize 
                  << " and sensitivity: " << sensitivity 
                  << " (higher = more tolerant)" << std::endl;
        std::cout << "Found " << diffRegions.size() << " differing regions in " 
                  << duration.count() << "ms." << std::endl;
        std::cout << "Advanced differences highlighted and saved to adv_differences_output.jpg" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

// Shows help information
void CLI::printHelp() const {
    std::cout << "Available commands:\n";
    std::cout << "  add <file_path>                                 Add an image file to the repository.\n";
    std::cout << "  commit                                          Commit the current changes.\n";
    std::cout << "  compare <v1> <v2> [sensitivity]                 Compare two versions using basic method.\n";
    std::cout << "                                                 Higher sensitivity (default 65) = less sensitive\n";
    std::cout << "  advcompare <v1> <v2> [chunkSize] [sensitivity]  Compare using advanced Merkle/Quadtree method.\n";
    std::cout << "                                                 Higher sensitivity (default 10) = more tolerant\n";
    std::cout << "  view <version>                                  View a specific version and display its image.\n";
    std::cout << "  delete <version>                               Delete a specific version.\n";
    std::cout << "  list                                           List all versions in the repository.\n";
    std::cout << "  help                                            Show this help message.\n";
    std::cout << "  exit                                            Exit the application.\n";
}
