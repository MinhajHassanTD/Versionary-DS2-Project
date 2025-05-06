#include "Quadtree.h"
#include <sstream>

// Constructor for QuadtreeNode
QuadtreeNode::QuadtreeNode(const cv::Rect& region, const cv::Mat& image)
    : region(region), topLeft(nullptr), topRight(nullptr), bottomLeft(nullptr), bottomRight(nullptr) {
    // Validate ROI dimensions with proper error message
    if (region.x < 0 || region.y < 0 || region.width <= 0 || region.height <= 0 ||
        region.x + region.width > image.cols || region.y + region.height > image.rows) {
        std::stringstream ss;
        ss << "Invalid ROI dimensions in QuadtreeNode. "
           << "ROI: [x=" << region.x << ", y=" << region.y 
           << ", width=" << region.width << ", height=" << region.height << "] "
           << "Image: [width=" << image.cols << ", height=" << image.rows << "]";
        throw std::invalid_argument(ss.str());
    }

    // Extract the chunk (ROI) from the image
    chunk = image(region).clone(); // Clone to ensure deep copy
}

// Checks if the node is a leaf (no children)
bool QuadtreeNode::isLeaf() const {
    return !topLeft && !topRight && !bottomLeft && !bottomRight;
}

// Subdivides the node into four children
void QuadtreeNode::subdivide() {
    // Calculate dimensions for child nodes
    int halfWidth = region.width / 2;
    int halfHeight = region.height / 2;
    
    // Handle odd dimensions properly
    int rightHalfWidth = region.width - halfWidth;
    int bottomHalfHeight = region.height - halfHeight;

    // Create child regions
    cv::Rect topLeftRegion(region.x, region.y, halfWidth, halfHeight);
    cv::Rect topRightRegion(region.x + halfWidth, region.y, rightHalfWidth, halfHeight);
    cv::Rect bottomLeftRegion(region.x, region.y + halfHeight, halfWidth, bottomHalfHeight);
    cv::Rect bottomRightRegion(region.x + halfWidth, region.y + halfHeight, rightHalfWidth, bottomHalfHeight);

    // Create child nodes if regions are valid
    if (isValidRegion(topLeftRegion)) {
        topLeft = std::make_shared<QuadtreeNode>(topLeftRegion, chunk);
    }
    if (isValidRegion(topRightRegion)) {
        topRight = std::make_shared<QuadtreeNode>(topRightRegion, chunk);
    }
    if (isValidRegion(bottomLeftRegion)) {
        bottomLeft = std::make_shared<QuadtreeNode>(bottomLeftRegion, chunk);
    }
    if (isValidRegion(bottomRightRegion)) {
        bottomRight = std::make_shared<QuadtreeNode>(bottomRightRegion, chunk);
    }
}

// Helper function to validate a region
bool QuadtreeNode::isValidRegion(const cv::Rect& region) const {
    return region.x >= 0 && region.y >= 0 &&
           region.width > 0 && region.height > 0 &&
           region.x + region.width <= chunk.cols &&
           region.y + region.height <= chunk.rows;
}

// Constructor for Quadtree
Quadtree::Quadtree(const cv::Mat& image, int minSize) : minSize(minSize) {
    // Validate the input image
    if (image.empty() || image.cols <= 0 || image.rows <= 0) {
        throw std::invalid_argument("Invalid image dimensions for Quadtree construction");
    }

    // Create the root node covering the entire image
    root = std::make_shared<QuadtreeNode>(cv::Rect(0, 0, image.cols, image.rows), image);

    // Build the tree recursively
    buildTree(root, minSize);
}

// Returns the root node of the Quadtree
std::shared_ptr<QuadtreeNode> Quadtree::getRoot() const {
    return root;
}

// Recursively builds the Quadtree
void Quadtree::buildTree(std::shared_ptr<QuadtreeNode> node, int minSize) {
    // Stop subdivision if the region is smaller than the minimum size
    if (node->region.width <= minSize || node->region.height <= minSize) {
        return;
    }

    // Subdivide the current node
    node->subdivide();

    // Recursively build the tree for each child
    if (node->topLeft) buildTree(node->topLeft, minSize);
    if (node->topRight) buildTree(node->topRight, minSize);
    if (node->bottomLeft) buildTree(node->bottomLeft, minSize);
    if (node->bottomRight) buildTree(node->bottomRight, minSize);
}
