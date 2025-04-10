#include "CLI.h"
#include "GUI.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    try {
        // Simple console test
        std::cout << "Versionary Test Output" << std::endl;
        
        // Create necessary directories
        if (!std::filesystem::exists("staging")) {
            std::filesystem::create_directory("staging");
            std::cout << "Created staging directory" << std::endl;
        }
        
        if (!std::filesystem::exists("versions")) {
            std::filesystem::create_directory("versions");
            std::cout << "Created versions directory" << std::endl;
        }
        
        std::cout << "Versionary - Simple CLI Mode" << std::endl;
        std::cout << "Available commands: add, view, compare, help, exit" << std::endl;
        
        std::string command;
        bool running = true;
        
        while (running) {
            std::cout << "Versionary> ";
            std::getline(std::cin, command);
            
            // Parse the command
            std::string cmd;
            std::string arg;
            
            size_t spacePos = command.find(' ');
            if (spacePos != std::string::npos) {
                cmd = command.substr(0, spacePos);
                arg = command.substr(spacePos + 1);
            } else {
                cmd = command;
            }
            
            // In the command parsing section, add the commit command:
            
            if (cmd == "exit") {
                std::cout << "Exiting Versionary..." << std::endl;
                running = false;
            }
            else if (cmd == "help") {
                std::cout << "Available commands:" << std::endl;
                std::cout << "  add <file_path> - Add an image file to the repository" << std::endl;
                std::cout << "  commit <filename> <version_name> - Move a file from staging to versions" << std::endl;
                std::cout << "  view <version> - View a specific version" << std::endl;
                std::cout << "  compare <v1> <v2> - Compare two versions" << std::endl;
                std::cout << "  help - Display this help message" << std::endl;
                std::cout << "  exit - Exit the application" << std::endl;
            }
            else if (cmd == "add") {
                if (arg.empty()) {
                    std::cout << "Error: Please specify a file path" << std::endl;
                } else {
                    std::cout << "Adding image: " << arg << std::endl;
                    
                    // Remove quotes if present
                    if ((arg.front() == '"' && arg.back() == '"') || 
                        (arg.front() == '\'' && arg.back() == '\'')) {
                        arg = arg.substr(1, arg.length() - 2);
                    }
                    
                    // Check if file exists with more detailed error reporting
                    try {
                        std::filesystem::path filePath(arg);
                        std::cout << "Checking path: " << filePath.string() << std::endl;
                        
                        if (!std::filesystem::exists(filePath)) {
                            std::cout << "Error: File does not exist: " << filePath.string() << std::endl;
                            std::cout << "Current working directory: " << std::filesystem::current_path().string() << std::endl;
                            std::cout << "Try using an absolute path or placing the file in: " 
                                      << std::filesystem::absolute(std::filesystem::current_path()).string() << std::endl;
                        } else {
                            try {
                                // Read the image with explicit flags
                                cv::Mat image = cv::imread(filePath.string(), cv::IMREAD_UNCHANGED);
                                if (image.empty()) {
                                    std::cout << "Error: Could not read image (file exists but may not be a valid image): " << filePath.string() << std::endl;
                                } else {
                                    // Save the image to staging area
                                    std::string filename = filePath.filename().string();
                                    std::string stagingPath = "staging/" + filename;
                                    
                                    // Ensure staging directory exists
                                    if (!std::filesystem::exists("staging")) {
                                        std::filesystem::create_directory("staging");
                                    }
                                    
                                    bool success = cv::imwrite(stagingPath, image);
                                    if (success) {
                                        std::cout << "Image added to staging area: " << stagingPath << std::endl;
                                        std::cout << "Image dimensions: " << image.cols << "x" << image.rows << std::endl;
                                    } else {
                                        std::cout << "Error: Failed to write image to staging area" << std::endl;
                                    }
                                }
                            } catch (const cv::Exception& e) {
                                std::cout << "OpenCV Error: " << e.what() << std::endl;
                            }
                        }
                    } catch (const std::exception& e) {
                        std::cout << "Error processing path: " << e.what() << std::endl;
                    }
                }
            }
            else if (!command.empty()) {
                std::cout << "Unknown command: " << cmd << std::endl;
                std::cout << "Type 'help' for a list of available commands" << std::endl;
            }
            else if (cmd == "commit") {
                // Parse the filename and version name
                std::string filename, versionName;
                size_t spacePos = arg.find(' ');
                if (spacePos != std::string::npos) {
                    filename = arg.substr(0, spacePos);
                    versionName = arg.substr(spacePos + 1);
                } else {
                    filename = arg;
                    versionName = filename; // Default version name is the same as filename
                }
                
                std::cout << "Committing file: " << filename << " as version: " << versionName << std::endl;
                
                // Construct paths
                std::string stagingPath = "staging/" + filename;
                std::string versionPath = "versions/" + versionName;
                
                // Check if file exists in staging
                if (!std::filesystem::exists(stagingPath)) {
                    std::cout << "Error: File does not exist in staging: " << stagingPath << std::endl;
                } else {
                    try {
                        // Ensure versions directory exists
                        if (!std::filesystem::exists("versions")) {
                            std::filesystem::create_directory("versions");
                        }
                        
                        // Copy file from staging to versions
                        std::filesystem::copy_file(stagingPath, versionPath, 
                                                  std::filesystem::copy_options::overwrite_existing);
                        std::cout << "File committed successfully as version: " << versionName << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "Error committing file: " << e.what() << std::endl;
                    }
                }
            }
            else if (cmd == "compare") {
                // Parse the two version arguments
                std::string version1, version2;
                size_t spacePos = arg.find(' ');
                if (spacePos != std::string::npos) {
                    version1 = arg.substr(0, spacePos);
                    version2 = arg.substr(spacePos + 1);
                    
                    std::cout << "Comparing versions: " << version1 << " and " << version2 << std::endl;
                    
                    // Construct paths to the version files
                    std::string path1 = "versions/" + version1;
                    std::string path2 = "versions/" + version2;
                    
                    // Check if both files exist
                    if (!std::filesystem::exists(path1)) {
                        std::cout << "Error: Version " << version1 << " does not exist" << std::endl;
                    } else if (!std::filesystem::exists(path2)) {
                        std::cout << "Error: Version " << version2 << " does not exist" << std::endl;
                    } else {
                        try {
                            // Load the images
                            cv::Mat img1 = cv::imread(path1, cv::IMREAD_UNCHANGED);
                            cv::Mat img2 = cv::imread(path2, cv::IMREAD_UNCHANGED);
                            
                            if (img1.empty() || img2.empty()) {
                                std::cout << "Error: Could not read one or both images" << std::endl;
                            } else {
                                // Check if images have the same dimensions
                                if (img1.size() != img2.size() || img1.type() != img2.type()) {
                                    std::cout << "Warning: Images have different dimensions or types" << std::endl;
                                    std::cout << "Image 1: " << img1.cols << "x" << img1.rows << " (type: " << img1.type() << ")" << std::endl;
                                    std::cout << "Image 2: " << img2.cols << "x" << img2.rows << " (type: " << img2.type() << ")" << std::endl;
                                }
                                
                                // Calculate difference
                                cv::Mat diff;
                                cv::absdiff(img1, img2, diff);
                                
                                // Calculate statistics
                                cv::Scalar meanDiff = cv::mean(diff);
                                double meanDiffValue = (meanDiff[0] + meanDiff[1] + meanDiff[2]) / 3.0;
                                
                                std::cout << "Comparison results:" << std::endl;
                                std::cout << "Mean difference: " << meanDiffValue << std::endl;
                                
                                if (meanDiffValue < 1.0) {
                                    std::cout << "The images are nearly identical" << std::endl;
                                } else if (meanDiffValue < 10.0) {
                                    std::cout << "The images have minor differences" << std::endl;
                                } else {
                                    std::cout << "The images have significant differences" << std::endl;
                                }
                                
                                // Save the difference image
                                std::string diffPath = "diff_" + version1 + "_" + version2 + ".png";
                                cv::imwrite(diffPath, diff);
                                std::cout << "Difference image saved to: " << diffPath << std::endl;
                            }
                        } catch (const cv::Exception& e) {
                            std::cout << "OpenCV Error: " << e.what() << std::endl;
                        }
                    }
                } else {
                    std::cout << "Error: Please specify two versions to compare (e.g., compare v1 v2)" << std::endl;
                }
            }
            else if (!command.empty()) {
                std::cout << "Unknown command: " << cmd << std::endl;
                std::cout << "Type 'help' for a list of available commands" << std::endl;
            }
            
            // Remove the duplicate compare code that's after this block
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "Press Enter to exit...";
        std::string dummy;
        std::getline(std::cin, dummy);
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        std::cout << "Press Enter to exit...";
        std::string dummy;
        std::getline(std::cin, dummy);
        return 1;
    }
    
    std::cout << "Program completed. Press Enter to exit...";
    std::string finalDummy;
    std::getline(std::cin, finalDummy);
    
    return 0;
}