
#include "GUI.h"
#include "Utils.h"               // Add this for Utils class
#include "ImageProcessor.h"      // Add this for ImageProcessor class
#include "ImageComparer.h"       // Add this for ImageComparer class
#include <iostream>
#include <stdexcept>             // Add this for std::runtime_error
#include <opencv2/highgui.hpp>   // Add this for cv::imshow, cv::waitKey, etc.
#include <opencv2/imgproc.hpp>   // Add this for cv::applyColorMap

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
        std::cout << "Adding image: " << filePath << "\n";
        
        // Check if file exists
        if (!Utils::fileExists(filePath)) {
            throw std::runtime_error("File does not exist: " + filePath);
        }
        
        // Read the image using ImageProcessor
        cv::Mat image = ImageProcessor::readImage(filePath);
        
        // Convert to grayscale for more efficient processing
        cv::Mat grayImage = ImageProcessor::convertToGrayscale(image);
        
        // Save the image to a staging area
        std::string stagingPath = "staging/" + filePath.substr(filePath.find_last_of("/\\") + 1);
        cv::imwrite(stagingPath, grayImage);
        
        std::cout << "Image added to staging area successfully.\n";
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

void GUI::handleCompareImages() {
    std::string version1, version2;
    std::cout << "Enter the first version to compare: ";
    std::cin >> version1;
    std::cout << "Enter the second version to compare: ";
    std::cin >> version2;

    try {
        std::cout << "Comparing versions: " << version1 << " and " << version2 << "\n";
        
        // Retrieve images for both versions
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
        
        // Display the difference image
        cv::namedWindow("Differences", cv::WINDOW_NORMAL);
        cv::Mat colorDiff;
        cv::applyColorMap(differences, colorDiff, cv::COLORMAP_JET);
        cv::imshow("Differences", colorDiff);
        cv::waitKey(0);
        cv::destroyWindow("Differences");
        
        std::cout << "Comparison complete. Differences saved to: " << outputPath << "\n";
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

void GUI::handleViewVersion() {
    std::string version;
    std::cout << "Enter the version to view: ";
    std::cin >> version;

    try {
        std::cout << "Viewing version: " << version << "\n";
        
        // Retrieve the image for the specified version
        std::string imagePath = "versions/" + version + "/image.jpg"; // Example path
        
        // Check if the version exists
        if (!Utils::fileExists(imagePath)) {
            throw std::runtime_error("Version does not exist: " + version);
        }
        
        // Read the image
        cv::Mat image = ImageProcessor::readImage(imagePath);
        
        // Display the image
        cv::namedWindow("Version: " + version, cv::WINDOW_NORMAL);
        cv::imshow("Version: " + version, image);
        cv::waitKey(0);
        cv::destroyWindow("Version: " + version);
        
        std::cout << "Version " << version << " displayed.\n";
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