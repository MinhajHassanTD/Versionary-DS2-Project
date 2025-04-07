#include <iostream>
#include <cassert>
#include <opencv2/opencv.hpp>
#include "Quadtree.h"

void testQuadtreeConstruction() {
    // Create a simple test image
    cv::Mat image(256, 256, CV_8UC1, cv::Scalar(0));
    
    // Draw some shapes to create non-homogeneous regions
    cv::rectangle(image, cv::Rect(50, 50, 100, 100), cv::Scalar(255), -1);
    cv::circle(image, cv::Point(200, 200), 30, cv::Scalar(128), -1);
    
    // Create Quadtree
    Quadtree quadtree(image, 4, 16, 10.0);
    
    // Get leaf nodes
    std::vector<std::shared_ptr<Quadtree::Node>> leafNodes = quadtree.getLeafNodes();
    
    // Should have some leaf nodes
    assert(!leafNodes.empty());
    
    // Get leaf hashes
    std::vector<std::string> leafHashes = quadtree.getLeafHashes();
    
    // Should have same number of hashes as leaf nodes
    assert(leafHashes.size() == leafNodes.size());
    
    std::cout << "Quadtree construction test passed!" << std::endl;
}

void testQuadtreeComparison() {
    // Create two similar test images with a difference
    cv::Mat image1(256, 256, CV_8UC1, cv::Scalar(0));
    cv::Mat image2(256, 256, CV_8UC1, cv::Scalar(0));
    
    // Draw common shapes
    cv::rectangle(image1, cv::Rect(50, 50, 100, 100), cv::Scalar(255), -1);
    cv::rectangle(image2, cv::Rect(50, 50, 100, 100), cv::Scalar(255), -1);
    
    // Draw different shapes
    cv::circle(image1, cv::Point(200, 200), 30, cv::Scalar(128), -1);
    cv::circle(image2, cv::Point(200, 200), 40, cv::Scalar(128), -1); // Different size
    
    // Create Quadtrees
    Quadtree quadtree1(image1, 4, 16, 10.0);
    Quadtree quadtree2(image2, 4, 16, 10.0);
    
    // Find different regions
    std::vector<cv::Rect> differentRegions = quadtree1.findDifferentRegions(quadtree2);
    
    // Should find at least one different region
    assert(!differentRegions.empty());
    
    std::cout << "Quadtree comparison test passed!" << std::endl;
}

void testQuadtreeVisualization() {
    // Create a simple test image
    cv::Mat image(256, 256, CV_8UC1, cv::Scalar(0));
    
    // Draw some shapes
    cv::rectangle(image, cv::Rect(50, 50, 100, 100), cv::Scalar(255), -1);
    cv::circle(image, cv::Point(200, 200), 30, cv::Scalar(128), -1);
    
    // Create Quadtree
    Quadtree quadtree(image, 4, 16, 10.0);
    
    // Visualize Quadtree
    cv::Mat visualization = quadtree.visualize(image);
    
    // Should not be empty
    assert(!visualization.empty());
    
    // Should be a color image (3 channels)
    assert(visualization.channels() == 3);
    
    std::cout << "Quadtree visualization test passed!" << std::endl;
}

int main() {
    std::cout << "Running Quadtree tests..." << std::endl;
    
    testQuadtreeConstruction();
    testQuadtreeComparison();
    testQuadtreeVisualization();
    
    std::cout << "All Quadtree tests passed!" << std::endl;
    
    return 0;
}
