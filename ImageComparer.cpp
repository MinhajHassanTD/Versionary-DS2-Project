#include "ImageComparer.h"
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Quadtree.h"
#include "MerkleTree.h"
#include <map>
#include <sstream>

// Compares two images and returns a matrix of differences
cv::Mat ImageComparer::compareImages(const cv::Mat& image1, const cv::Mat& image2) {
    // Check if images are valid
    if (image1.empty() || image2.empty()) {
        throw std::runtime_error("One or both images are empty");
    }

    // Create a copy of the first image to overlay differences on
    cv::Mat result;
    
    // Ensure the image is in color for highlighting differences
    if (image1.channels() == 1) {
        cv::cvtColor(image1, result, cv::COLOR_GRAY2BGR);
    } else {
        image1.copyTo(result);
    }
    
    // If images have different sizes, resize the second one to match
    cv::Mat resizedImage2;
    if (image1.size() != image2.size()) {
        cv::resize(image2, resizedImage2, image1.size());
    } else {
        resizedImage2 = image2;
    }
    
    // Convert both to grayscale for comparison
    cv::Mat gray1, gray2;
    if (image1.channels() == 3 || image1.channels() == 4) {
        cv::cvtColor(image1, gray1, cv::COLOR_BGR2GRAY);
    } else {
        gray1 = image1;
    }
    
    if (resizedImage2.channels() == 3 || resizedImage2.channels() == 4) {
        cv::cvtColor(resizedImage2, gray2, cv::COLOR_BGR2GRAY);
    } else {
        gray2 = resizedImage2;
    }
    
    // Calculate absolute difference
    cv::Mat diffMap;
    cv::absdiff(gray1, gray2, diffMap);
    
    // Apply threshold to identify significant differences
    cv::Mat thresholdedDiff;
    cv::threshold(diffMap, thresholdedDiff, 30, 255, cv::THRESH_BINARY);
    
    // Find contours of differences
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresholdedDiff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Draw contours on the original image with a semi-transparent overlay
    for (const auto& contour : contours) {
        // Filter small noise contours (optional)
        if (cv::contourArea(contour) > 10) {
            // Draw filled contour with semi-transparency
            cv::Rect boundingRect = cv::boundingRect(contour);
            
            // Create a mask for the current contour
            cv::Mat mask = cv::Mat::zeros(result.size(), CV_8UC1);
            cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), -1);
            
            // Create a colored overlay
            cv::Mat overlay = cv::Mat::zeros(result.size(), result.type());
            overlay.setTo(cv::Scalar(0, 0, 255), mask); // Red color
            
            // Apply the overlay with transparency
            cv::addWeighted(result, 1.0, overlay, 0.5, 0, result);
            
            // Draw a border around the difference area
            cv::rectangle(result, boundingRect, cv::Scalar(0, 255, 0), 2);
        }
    }
    
    return result;
}

// Enhanced comparison using Quadtree and Merkle Tree structures
std::vector<cv::Rect> ImageComparer::compareWithStructures(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize) {
    std::vector<cv::Rect> diffRegions;
    
    try {
        // Process both images with Quadtree decomposition
        Quadtree quadtree1(image1, minChunkSize);
        Quadtree quadtree2(image2, minChunkSize);
        
        // Collect hashes from both quadtrees
        std::vector<std::string> hashes1;
        std::vector<std::string> hashes2;
        std::map<std::string, cv::Rect> hashToRegion1;
        std::map<std::string, cv::Rect> hashToRegion2;
        
        // Collect hashes and build mapping from hash to region
        collectHashesWithRegions(quadtree1.getRoot(), hashes1, hashToRegion1);
        collectHashesWithRegions(quadtree2.getRoot(), hashes2, hashToRegion2);
        
        // Build Merkle Trees
        MerkleTree tree1(hashes1);
        MerkleTree tree2(hashes2);
        
        // Compare root hashes
        if (tree1.getRootHash() == tree2.getRootHash()) {
            std::cout << "Images are identical according to Merkle Tree comparison" << std::endl;
            return diffRegions; // Empty, no differences
        }
        
        // Find differences by comparing all hashes
        for (const auto& pair : hashToRegion1) {
            const std::string& hash = pair.first;
            const cv::Rect& region = pair.second;
            
            // If this hash from image1 doesn't exist in image2 hashes, it's different
            if (std::find(hashes2.begin(), hashes2.end(), hash) == hashes2.end()) {
                diffRegions.push_back(region);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in compareWithStructures: " << e.what() << std::endl;
    }
    
    return diffRegions;
}

// Helper function to collect hashes and map them to regions
void ImageComparer::collectHashesWithRegions(std::shared_ptr<QuadtreeNode> node, 
                                            std::vector<std::string>& hashes, 
                                            std::map<std::string, cv::Rect>& hashToRegion) {
    if (!node) return;
    
    if (node->isLeaf()) {
        // Hash the image chunk data
        std::string hash = hashImageChunk(node->chunk);
        hashes.push_back(hash);
        hashToRegion[hash] = node->region;
    } else {
        // Recursively collect hashes from children
        collectHashesWithRegions(node->topLeft, hashes, hashToRegion);
        collectHashesWithRegions(node->topRight, hashes, hashToRegion);
        collectHashesWithRegions(node->bottomLeft, hashes, hashToRegion);
        collectHashesWithRegions(node->bottomRight, hashes, hashToRegion);
    }
}

// Hash function (same as in CLI.cpp and GUI.cpp)
std::string ImageComparer::hashImageChunk(const cv::Mat& chunk) {
    cv::Scalar mean = cv::mean(chunk);
    std::stringstream ss;
    ss << "chunk_" << mean[0];
    return ss.str();
}

// Visualizes the differences and saves the result to a file
void ImageComparer::visualizeDifferences(const cv::Mat& differences, const std::string& outputPath) {
    // Check if differences matrix is valid
    if (differences.empty()) {
        throw std::runtime_error("Differences matrix is empty");
    }
    
    // Save the result directly (no additional processing)
    if (!cv::imwrite(outputPath, differences)) {
        throw std::runtime_error("Failed to save the differences to " + outputPath);
    }
    
    std::cout << "Differences visualized and saved to " << outputPath << "\n";
}

// Highlight specific regions of difference on the original image
void ImageComparer::highlightDifferences(const cv::Mat& image, const std::vector<cv::Rect>& diffRegions, 
                                        const std::string& outputPath) {
    if (image.empty()) {
        throw std::runtime_error("Image is empty");
    }
    
    // Create a copy of the image to draw on
    cv::Mat result;
    if (image.channels() == 1) {
        cv::cvtColor(image, result, cv::COLOR_GRAY2BGR); // Convert to color for highlighting
    } else {
        image.copyTo(result);
    }
    
    // Draw rectangles around difference regions
    for (const auto& region : diffRegions) {
        // Create a semi-transparent overlay for the region
        cv::Mat overlay = result(region).clone();
        cv::addWeighted(overlay, 0.5, cv::Scalar(0, 0, 255), 0.5, 0, overlay);
        overlay.copyTo(result(region));
        
        // Draw a green border around the difference area
        cv::rectangle(result, region, cv::Scalar(0, 255, 0), 2);
    }
    
    // Save the result
    if (!cv::imwrite(outputPath, result)) {
        throw std::runtime_error("Failed to save the highlighted image to " + outputPath);
    }
    
    std::cout << "Differences highlighted and saved to " << outputPath << "\n";
}
//new
