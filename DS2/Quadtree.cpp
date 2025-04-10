#include "Quadtree.h"
#include <stdexcept>
#include <algorithm> // For std::max

// Constructor for QuadtreeNode
QuadtreeNode::QuadtreeNode(const cv::Rect& region, const cv::Mat& image)
    : region(region), chunk(image(region)), topLeft(nullptr), topRight(nullptr), bottomLeft(nullptr), bottomRight(nullptr) {}

// Checks if the node is a leaf (no children)
bool QuadtreeNode::isLeaf() const {
    return !topLeft && !topRight && !bottomLeft && !bottomRight;
}

// Subdivides the node into four children
void QuadtreeNode::subdivide() {
    int halfWidth = region.width / 2;
    int halfHeight = region.height / 2;

    topLeft = std::make_shared<QuadtreeNode>(cv::Rect(region.x, region.y, halfWidth, halfHeight), chunk);
    topRight = std::make_shared<QuadtreeNode>(cv::Rect(region.x + halfWidth, region.y, halfWidth, halfHeight), chunk);
    bottomLeft = std::make_shared<QuadtreeNode>(cv::Rect(region.x, region.y + halfHeight, halfWidth, halfHeight), chunk);
    bottomRight = std::make_shared<QuadtreeNode>(cv::Rect(region.x + halfWidth, region.y + halfHeight, halfWidth, halfHeight), chunk);
}

// Constructor for Quadtree with image
Quadtree::Quadtree(const cv::Mat& image, int minSize) : minSize(minSize), minChunkSize(minSize), root(nullptr) {
    if (minSize <= 0) {
        throw std::invalid_argument("Minimum chunk size must be positive");
    }
    root = std::make_shared<QuadtreeNode>(cv::Rect(0, 0, image.cols, image.rows), image);
    buildTree(root, minSize);
}

// Constructor for Quadtree with just minSize
Quadtree::Quadtree(int minSize) : minSize(minSize), minChunkSize(minSize), root(nullptr) {
    if (minSize <= 0) {
        throw std::invalid_argument("Minimum chunk size must be positive");
    }
}

// Returns the root node of the Quadtree
std::shared_ptr<QuadtreeNode> Quadtree::getRoot() const {
    return root;
}

// Recursively builds the Quadtree
void Quadtree::buildTree(std::shared_ptr<QuadtreeNode> node, int minSize) {
    if (!node || node->region.width <= minSize || node->region.height <= minSize) {
        return;
    }

    node->subdivide();

    buildTree(node->topLeft, minSize);
    buildTree(node->topRight, minSize);
    buildTree(node->bottomLeft, minSize);
    buildTree(node->bottomRight, minSize);
}

std::vector<cv::Mat> Quadtree::divideImage(const cv::Mat& image) {
    if (image.empty()) {
        throw std::runtime_error("Cannot divide an empty image");
    }
    
    std::vector<cv::Mat> chunks;
    divideRecursively(image, chunks, 0, 0, image.cols, image.rows);
    return chunks;
}

void Quadtree::divideRecursively(const cv::Mat& image, std::vector<cv::Mat>& chunks, 
                                int x, int y, int width, int height) {
    // If the chunk is already small enough or is homogeneous, add it to the list
    if (width <= minChunkSize || height <= minChunkSize || isHomogeneous(image, x, y, width, height)) {
        cv::Mat chunk = image(cv::Rect(x, y, width, height)).clone();
        chunks.push_back(chunk);
        return;
    }
    
    // Otherwise, divide into four quadrants
    int halfWidth = width / 2;
    int halfHeight = height / 2;
    
    // Top-left
    divideRecursively(image, chunks, x, y, halfWidth, halfHeight);
    
    // Top-right
    divideRecursively(image, chunks, x + halfWidth, y, width - halfWidth, halfHeight);
    
    // Bottom-left
    divideRecursively(image, chunks, x, y + halfHeight, halfWidth, height - halfHeight);
    
    // Bottom-right
    divideRecursively(image, chunks, x + halfWidth, y + halfHeight, width - halfWidth, height - halfHeight);
}

bool Quadtree::isHomogeneous(const cv::Mat& image, int x, int y, int width, int height) {
    cv::Mat chunk = image(cv::Rect(x, y, width, height));
    
    // For grayscale images
    if (chunk.channels() == 1) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(chunk, mean, stddev);
        
        // If standard deviation is low, consider the chunk homogeneous
        return stddev[0] < 10.0; // Threshold can be adjusted
    }
    // For color images
    else {
        cv::Mat grayChunk;
        cv::cvtColor(chunk, grayChunk, cv::COLOR_BGR2GRAY);
        
        cv::Scalar mean, stddev;
        cv::meanStdDev(grayChunk, mean, stddev);
        
        return stddev[0] < 10.0; // Threshold can be adjusted
    }
}

cv::Mat Quadtree::reconstructImage(const std::vector<cv::Mat>& chunks, int originalWidth, int originalHeight) {
    // This is a simplified implementation that assumes chunks are arranged in a grid
    // A more sophisticated implementation would need to track the position of each chunk
    
    if (chunks.empty()) {
        throw std::runtime_error("Cannot reconstruct image from empty chunks");
    }
    
    cv::Mat result = cv::Mat::zeros(originalHeight, originalWidth, chunks[0].type());
    
    int currentX = 0;
    int currentY = 0;
    int maxHeight = 0;
    
    for (const auto& chunk : chunks) {
        // If adding this chunk would exceed the width, move to the next row
        if (currentX + chunk.cols > originalWidth) {
            currentX = 0;
            currentY += maxHeight;
            maxHeight = 0;
        }
        
        // Copy the chunk to the result image
        chunk.copyTo(result(cv::Rect(currentX, currentY, chunk.cols, chunk.rows)));
        
        // Update position
        currentX += chunk.cols;
        maxHeight = std::max(maxHeight, chunk.rows);
    }
    
    return result;
}
