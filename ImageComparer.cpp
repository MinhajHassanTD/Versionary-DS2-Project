#include "ImageComparer.h"
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Quadtree.h"
#include "MerkleTree.h"
#include "Utils.h"
#include <map>
#include <sstream>
#include <unordered_set>

// Basic pixel-by-pixel comparison of two images
// Returns an image highlighting the differences
cv::Mat ImageComparer::compareImages(const cv::Mat& image1, const cv::Mat& image2, int sensitivity) {
    if (image1.empty() || image2.empty()) {
        throw std::runtime_error("One or both images are empty");
    }

    cv::Mat result;
    
    if (image1.channels() == 1) {
        cv::cvtColor(image1, result, cv::COLOR_GRAY2BGR);
    } else {
        image1.copyTo(result);
    }
    
    // Resize second image if dimensions don't match
    cv::Mat resizedImage2;
    if (image1.size() != image2.size()) {
        cv::resize(image2, resizedImage2, image1.size());
    } else {
        resizedImage2 = image2;
    }
    
    // Convert both images to grayscale
    cv::Mat gray1, gray2;
    if (image1.channels() == 3 || image1.channels() == 4) {
        cv::cvtColor(image1, gray1, cv::COLOR_BGR2GRAY);
    } else {
        gray1 = image1.clone();
    }
    
    if (resizedImage2.channels() == 3 || resizedImage2.channels() == 4) {
        cv::cvtColor(resizedImage2, gray2, cv::COLOR_BGR2GRAY);
    } else {
        gray2 = resizedImage2.clone();
    }
    
    // Apply blur to reduce noise
    cv::GaussianBlur(gray1, gray1, cv::Size(3, 3), 0);
    cv::GaussianBlur(gray2, gray2, cv::Size(3, 3), 0);
    
    // Calculate absolute difference between images
    cv::Mat diffMap;
    cv::absdiff(gray1, gray2, diffMap);
    
    // Apply threshold to identify significant differences
    cv::Mat thresholdedDiff;
    cv::threshold(diffMap, thresholdedDiff, sensitivity, 255, cv::THRESH_BINARY);
    
    // Clean up the difference map using morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(thresholdedDiff, thresholdedDiff, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(thresholdedDiff, thresholdedDiff, cv::MORPH_OPEN, kernel);
    
    // Find contours in the difference map
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresholdedDiff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Highlight contours above minimum size
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > 100) {
            cv::Rect boundingRect = cv::boundingRect(contour);
            
            cv::Mat mask = cv::Mat::zeros(result.size(), CV_8UC1);
            cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, 0, cv::Scalar(255), -1);
            
            cv::Mat overlay = cv::Mat::zeros(result.size(), result.type());
            overlay.setTo(cv::Scalar(0, 0, 255), mask);
            
            cv::addWeighted(result, 1.0, overlay, 0.5, 0, result);
            
            cv::rectangle(result, boundingRect, cv::Scalar(0, 255, 0), 2);
        }
    }
    
    // Collect bounding rectangles for merging
    std::vector<cv::Rect> boundingRects;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) > 100) {
            boundingRects.push_back(cv::boundingRect(contour));
        }
    }

    // Merge overlapping rectangles for cleaner visualization
    std::vector<cv::Rect> mergedRects;
    std::vector<bool> processed(boundingRects.size(), false);

    for (size_t i = 0; i < boundingRects.size(); i++) {
        if (processed[i]) continue;
        
        cv::Rect current = boundingRects[i];
        processed[i] = true;
        
        bool changes;
        do {
            changes = false;
            for (size_t j = 0; j < boundingRects.size(); j++) {
                if (processed[j]) continue;
                
                cv::Rect intersection = current & boundingRects[j];
                if (!intersection.empty()) {
                    current = current | boundingRects[j];
                    processed[j] = true;
                    changes = true;
                }
            }
        } while (changes);
        
        mergedRects.push_back(current);
    }

    // Draw the merged rectangles
    for (const auto& rect : mergedRects) {
        cv::Mat mask = cv::Mat::zeros(result.size(), CV_8UC1);
        cv::rectangle(mask, rect, cv::Scalar(255), -1);
        
        cv::Mat overlay = cv::Mat::zeros(result.size(), result.type());
        overlay.setTo(cv::Scalar(0, 0, 255), mask);
        
        cv::addWeighted(result, 1.0, overlay, 0.5, 0, result);
        
        cv::rectangle(result, rect, cv::Scalar(0, 255, 0), 2);
    }

    return result;
}

// Check if two perceptual hashes are within a similarity threshold
bool ImageComparer::areHashesSimilar(const std::string& hash1, const std::string& hash2, int threshold) {
    int distance = Utils::hammingDistance(hash1, hash2);
    return distance != -1 && distance <= threshold;
}

// Advanced comparison using Quadtree and MerkleTree structures
// Uses a hybrid approach of structural comparison followed by pixel analysis
std::vector<cv::Rect> ImageComparer::compareWithStructures(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize, int sensitivity) {
    std::vector<cv::Rect> diffRegions;
    
    try {
        // Resize second image if dimensions don't match
        cv::Mat resizedImage2;
        if (image1.size() != image2.size()) {
            cv::resize(image2, resizedImage2, image1.size());
        } else {
            resizedImage2 = image2;
        }
        
        // Convert both images to grayscale
        cv::Mat gray1, gray2;
        if (image1.channels() == 3 || image1.channels() == 4) {
            cv::cvtColor(image1, gray1, cv::COLOR_BGR2GRAY);
        } else {
            gray1 = image1.clone();
        }
        
        if (resizedImage2.channels() == 3 || resizedImage2.channels() == 4) {
            cv::cvtColor(resizedImage2, gray2, cv::COLOR_BGR2GRAY);
        } else {
            gray2 = resizedImage2.clone();
        }
        
        // Apply blur to reduce noise
        cv::GaussianBlur(gray1, gray1, cv::Size(3, 3), 0);
        cv::GaussianBlur(gray2, gray2, cv::Size(3, 3), 0);
        
        // PHASE 1: Use Quadtree/Merkle Tree to identify suspect regions
        Quadtree quadtree1(gray1, minChunkSize);
        Quadtree quadtree2(gray2, minChunkSize);
        
        std::vector<std::string> hashes1;
        std::vector<std::string> hashes2;
        std::map<std::string, cv::Rect> hashToRegion1;
        std::map<std::string, cv::Rect> hashToRegion2;
        
        collectHashesWithRegions(quadtree1.getRoot(), hashes1, hashToRegion1);
        collectHashesWithRegions(quadtree2.getRoot(), hashes2, hashToRegion2);
        
        MerkleTree tree1(hashes1);
        MerkleTree tree2(hashes2);
        
        // Quick exit if images are identical
        if (tree1.getRootHash() == tree2.getRootHash()) {
            return diffRegions;
        }
        
        // Create hash set for faster lookups
        std::unordered_set<std::string> hashSet2;
        for (const auto& hash : hashes2) {
            hashSet2.insert(hash);
        }
        
        // Find potentially different regions by comparing hashes
        std::vector<cv::Rect> suspectRegions;
        int similarityThreshold = sensitivity;
        
        for (const auto& pair : hashToRegion1) {
            const std::string& hash = pair.first;
            const cv::Rect& region = pair.second;
            
            // Skip if exact match exists
            if (hashSet2.find(hash) != hashSet2.end()) {
                continue;
            }
            
            // Check for similar hashes within threshold
            bool foundSimilar = false;
            for (const auto& h2 : hashes2) {
                if (areHashesSimilar(hash, h2, similarityThreshold)) {
                    foundSimilar = true;
                    break;
                }
            }
            
            if (!foundSimilar) {
                suspectRegions.push_back(region);
            }
        }
        
        // PHASE 2: Refine suspect regions with pixel-level analysis
        for (const auto& region : suspectRegions) {
            // Ensure region is within image bounds
            cv::Rect safeRegion = region & cv::Rect(0, 0, gray1.cols, gray1.rows);
            if (safeRegion.width <= 0 || safeRegion.height <= 0) continue;
            
            // Extract region from both images
            cv::Mat region1 = gray1(safeRegion);
            cv::Mat region2 = gray2(safeRegion);
            
            // Calculate pixel differences in this region
            cv::Mat diffMap;
            cv::absdiff(region1, region2, diffMap);
            
            cv::Mat thresholdedDiff;
            cv::threshold(diffMap, thresholdedDiff, 45, 255, cv::THRESH_BINARY);
            
            // Clean up the difference map
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::morphologyEx(thresholdedDiff, thresholdedDiff, cv::MORPH_CLOSE, kernel);
            cv::morphologyEx(thresholdedDiff, thresholdedDiff, cv::MORPH_OPEN, kernel);
            
            // Find contours of actual differences
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(thresholdedDiff, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            
            // Add significant contours to difference regions
            for (const auto& contour : contours) {
                if (cv::contourArea(contour) > 25) {
                    cv::Rect contourRect = cv::boundingRect(contour);
                    // Translate back to original image coordinates
                    contourRect.x += safeRegion.x;
                    contourRect.y += safeRegion.y;
                    diffRegions.push_back(contourRect);
                }
            }
            
            // If no significant contours, add small indicator
            if (contours.empty()) {
                int centerX = safeRegion.x + safeRegion.width/2 - 5;
                int centerY = safeRegion.y + safeRegion.height/2 - 5;
                diffRegions.push_back(cv::Rect(centerX, centerY, 10, 10));
            }
        }
        
        // Merge close or overlapping regions
        if (!diffRegions.empty()) {
            std::vector<cv::Rect> mergedRegions;
            std::vector<bool> processed(diffRegions.size(), false);
            
            for (size_t i = 0; i < diffRegions.size(); i++) {
                if (processed[i]) continue;
                
                cv::Rect current = diffRegions[i];
                processed[i] = true;
                
                bool changes;
                do {
                    changes = false;
                    for (size_t j = 0; j < diffRegions.size(); j++) {
                        if (processed[j]) continue;
                        
                        // Slightly expanded rectangle to detect nearby regions
                        cv::Rect expanded(
                            current.x - 5, current.y - 5,
                            current.width + 10, current.height + 10
                        );
                        
                        if ((expanded & diffRegions[j]).area() > 0) {
                            current = current | diffRegions[j];
                            processed[j] = true;
                            changes = true;
                        }
                    }
                } while (changes);
                
                mergedRegions.push_back(current);
            }
            
            return mergedRegions;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in compareWithStructures: " << e.what() << std::endl;
    }
    
    return diffRegions;
}

// Recursively collect perceptual hashes from quadtree leaf nodes
void ImageComparer::collectHashesWithRegions(std::shared_ptr<QuadtreeNode> node, 
                                          std::vector<std::string>& hashes, 
                                          std::map<std::string, cv::Rect>& hashToRegion) {
    if (!node) return;
    
    if (node->isLeaf()) {
        std::string hash = hashImageChunk(node->chunk);
        hashes.push_back(hash);
        hashToRegion[hash] = node->region;
    } else {
        // Recursively process child nodes
        collectHashesWithRegions(node->topLeft, hashes, hashToRegion);
        collectHashesWithRegions(node->topRight, hashes, hashToRegion);
        collectHashesWithRegions(node->bottomLeft, hashes, hashToRegion);
        collectHashesWithRegions(node->bottomRight, hashes, hashToRegion);
    }
}

// Generate perceptual hash for an image chunk
std::string ImageComparer::hashImageChunk(const cv::Mat& chunk) {
    return Utils::computePerceptualHash(chunk);
}

// Save difference visualization to file
void ImageComparer::visualizeDifferences(const cv::Mat& differences, const std::string& outputPath) {
    if (differences.empty()) {
        throw std::runtime_error("Differences matrix is empty");
    }
    
    if (!cv::imwrite(outputPath, differences)) {
        throw std::runtime_error("Failed to save the differences to " + outputPath);
    }
}

// Highlight difference regions on the original image and save to file
void ImageComparer::highlightDifferences(const cv::Mat& image, const std::vector<cv::Rect>& diffRegions, 
                                        const std::string& outputPath) {
    if (image.empty()) {
        throw std::runtime_error("Image is empty");
    }
    
    cv::Mat result;
    if (image.channels() == 1) {
        cv::cvtColor(image, result, cv::COLOR_GRAY2BGR);
    } else {
        image.copyTo(result);
    }
    
    // Apply transparent red overlay and green rectangle for each difference region
    for (const auto& region : diffRegions) {
        cv::Mat overlay = result(region).clone();
        cv::addWeighted(overlay, 0.5, cv::Scalar(0, 0, 255), 0.5, 0, overlay);
        overlay.copyTo(result(region));
        
        cv::rectangle(result, region, cv::Scalar(0, 255, 0), 2);
    }
    
    if (!cv::imwrite(outputPath, result)) {
        throw std::runtime_error("Failed to save the highlighted image to " + outputPath);
    }
}