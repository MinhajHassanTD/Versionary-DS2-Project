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

    // Create a blank difference map (white = no difference, black = difference)
    cv::Mat diffMap;
    
    // If images have different sizes, resize the smaller one
    if (image1.size() != image2.size()) {
        cv::Mat resized;
        if (image1.total() < image2.total()) {
            cv::resize(image1, resized, image2.size());
            cv::absdiff(resized, image2, diffMap);
        } else {
            cv::resize(image2, resized, image1.size());
            cv::absdiff(image1, resized, diffMap);
        }
    } else {
        // Calculate absolute difference
        cv::absdiff(image1, image2, diffMap);
    }
    
    return diffMap;
}

// Enhanced comparison using Quadtree and Merkle Tree structures
std::vector<cv::Rect> ImageComparer::compareWithStructures(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize) {
    std::vector<cv::Rect> diffRegions;
    
    // Process both images with Quadtree decomposition
    Quadtree quadtree1(image1, minChunkSize);
    Quadtree quadtree2(image2, minChunkSize);
    
    // Collect hashes from both quadtrees
    std::vector<std::string> hashes1;
    std::vector<std::string> hashes2;
    std::map<std::string, cv::Rect> hashToRegion1;
    
    // Collect hashes and build mapping from hash to region
    collectHashesWithRegions(quadtree1.getRoot(), hashes1, hashToRegion1);
    
    // Build Merkle Trees
    MerkleTree tree1(hashes1);
    MerkleTree tree2(hashes2);
    
    // Compare root hashes
    if (tree1.getRootHash() == tree2.getRootHash()) {
        std::cout << "Images are identical according to Merkle Tree comparison" << std::endl;
        return diffRegions; // Empty no differences
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

    // Apply a threshold to make differences more visible
    cv::Mat thresholded;
    cv::threshold(differences, thresholded, 30, 255, cv::THRESH_BINARY);
    
    // Create a colorized version of the differences
    cv::Mat colorDiff;
    
    // Apply colormap to make differences more visible (check if input is grayscale first)
    if (thresholded.channels() == 1) {
        cv::applyColorMap(thresholded, colorDiff, cv::COLORMAP_JET);
    } else {
        // If already a color image, use it directly
        colorDiff = thresholded;
    }

    // Save the result
    if (!cv::imwrite(outputPath, colorDiff)) {
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
        cv::rectangle(result, region, cv::Scalar(0, 0, 255), 2); // Red rectangle
    }
    
    // Save the result
    if (!cv::imwrite(outputPath, result)) {
        throw std::runtime_error("Failed to save the highlighted image to " + outputPath);
    }
    
    std::cout << "Differences highlighted and saved to " << outputPath << "\n";
}
