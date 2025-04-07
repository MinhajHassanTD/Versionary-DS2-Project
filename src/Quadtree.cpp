#include "Quadtree.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <queue>

// Helper function to compute SHA-256 hash
std::string sha256_qt(const std::vector<uchar>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

Quadtree::Quadtree(const cv::Mat& image, int maxDepth, int minSize, double threshold)
    : image_(image.clone()), maxDepth_(maxDepth), minSize_(minSize), threshold_(threshold) {

    // Create root node covering the entire image
    cv::Rect rootRegion(0, 0, image.cols, image.rows);
    root_ = std::make_shared<Node>(rootRegion);

    // Build the tree recursively
    buildTree(root_, 0);

    // Collect all leaf nodes
    collectLeafNodes(root_, leafNodes_);
}

Quadtree::Quadtree(const cv::Mat& image, std::shared_ptr<Node> rootNode, int maxDepth, int minSize, double threshold)
    : image_(image.clone()), root_(rootNode), maxDepth_(maxDepth), minSize_(minSize), threshold_(threshold) {

    // Collect all leaf nodes
    collectLeafNodes(root_, leafNodes_);
}

std::shared_ptr<Quadtree::Node> Quadtree::getRoot() const {
    return root_;
}

std::vector<std::shared_ptr<Quadtree::Node>> Quadtree::getLeafNodes() const {
    return leafNodes_;
}

std::vector<std::string> Quadtree::getLeafHashes() const {
    std::vector<std::string> hashes;
    hashes.reserve(leafNodes_.size());

    for (const auto& node : leafNodes_) {
        hashes.push_back(node->hash);
    }

    return hashes;
}

cv::Mat Quadtree::visualize(const cv::Mat& image) const {
    cv::Mat visualization = image.clone();

    // Convert to color if grayscale
    if (visualization.channels() == 1) {
        cv::cvtColor(visualization, visualization, cv::COLOR_GRAY2BGR);
    }

    // Draw all nodes recursively
    drawNode(visualization, root_);

    return visualization;
}

std::vector<cv::Rect> Quadtree::findDifferentRegions(const Quadtree& other) const {
    std::vector<cv::Rect> differentRegions;

    // Compare trees recursively
    findDifferentRegionsRecursive(root_, other.getRoot(), differentRegions);

    return differentRegions;
}

void Quadtree::buildTree(std::shared_ptr<Node> node, int depth) {
    // Stop if maximum depth reached or region is too small
    if (depth >= maxDepth_ ||
        node->region.width <= minSize_ ||
        node->region.height <= minSize_ ||
        isHomogeneous(node->region)) {

        // Compute hash for leaf node
        node->hash = computeHash(node->region);
        node->isLeaf = true;
        return;
    }

    // Split region into four quadrants
    int halfWidth = node->region.width / 2;
    int halfHeight = node->region.height / 2;

    cv::Rect topLeftRegion(node->region.x, node->region.y, halfWidth, halfHeight);
    cv::Rect topRightRegion(node->region.x + halfWidth, node->region.y,
                           node->region.width - halfWidth, halfHeight);
    cv::Rect bottomLeftRegion(node->region.x, node->region.y + halfHeight,
                             halfWidth, node->region.height - halfHeight);
    cv::Rect bottomRightRegion(node->region.x + halfWidth, node->region.y + halfHeight,
                              node->region.width - halfWidth, node->region.height - halfHeight);

    // Create child nodes
    node->topLeft = std::make_shared<Node>(topLeftRegion);
    node->topRight = std::make_shared<Node>(topRightRegion);
    node->bottomLeft = std::make_shared<Node>(bottomLeftRegion);
    node->bottomRight = std::make_shared<Node>(bottomRightRegion);

    // Mark as non-leaf
    node->isLeaf = false;

    // Recursively build subtrees
    buildTree(node->topLeft, depth + 1);
    buildTree(node->topRight, depth + 1);
    buildTree(node->bottomLeft, depth + 1);
    buildTree(node->bottomRight, depth + 1);

    // Compute hash for internal node by combining child hashes
    node->hash = sha256_qt({
        node->topLeft->hash.begin(), node->topLeft->hash.end(),
        node->topRight->hash.begin(), node->topRight->hash.end(),
        node->bottomLeft->hash.begin(), node->bottomLeft->hash.end(),
        node->bottomRight->hash.begin(), node->bottomRight->hash.end()
    });
}

bool Quadtree::isHomogeneous(const cv::Rect& region) const {
    cv::Mat roi = image_(region);
    cv::Scalar mean, stddev;
    cv::meanStdDev(roi, mean, stddev);

    // Check if standard deviation is below threshold
    double maxStdDev = 0;
    for (int i = 0; i < stddev.channels; i++) {
        maxStdDev = std::max(maxStdDev, stddev[i]);
    }

    return maxStdDev < threshold_;
}

std::string Quadtree::computeHash(const cv::Rect& region) const {
    cv::Mat roi = image_(region);
    std::vector<uchar> buffer;
    cv::imencode(".png", roi, buffer);
    return sha256_qt(buffer);
}

void Quadtree::collectLeafNodes(const std::shared_ptr<Node>& node,
                              std::vector<std::shared_ptr<Node>>& leafNodes) const {
    if (!node) {
        return;
    }

    if (node->isLeaf) {
        leafNodes.push_back(node);
    } else {
        collectLeafNodes(node->topLeft, leafNodes);
        collectLeafNodes(node->topRight, leafNodes);
        collectLeafNodes(node->bottomLeft, leafNodes);
        collectLeafNodes(node->bottomRight, leafNodes);
    }
}

void Quadtree::drawNode(cv::Mat& image, const std::shared_ptr<Node>& node) const {
    if (!node) {
        return;
    }

    // Draw rectangle for current node
    cv::rectangle(image, node->region, cv::Scalar(0, 255, 0), 1);

    // Recursively draw child nodes
    if (!node->isLeaf) {
        drawNode(image, node->topLeft);
        drawNode(image, node->topRight);
        drawNode(image, node->bottomLeft);
        drawNode(image, node->bottomRight);
    }
}

void Quadtree::findDifferentRegionsRecursive(const std::shared_ptr<Node>& node1,
                                          const std::shared_ptr<Node>& node2,
                                          std::vector<cv::Rect>& regions) const {
    // If hashes are the same, no differences in this subtree
    if (node1->hash == node2->hash) {
        return;
    }

    // If both are leaf nodes, add region to differences
    if (node1->isLeaf && node2->isLeaf) {
        regions.push_back(node1->region);
        return;
    }

    // If one is leaf and other is not, or if both are internal nodes,
    // recursively check children
    if (!node1->isLeaf && !node2->isLeaf) {
        findDifferentRegionsRecursive(node1->topLeft, node2->topLeft, regions);
        findDifferentRegionsRecursive(node1->topRight, node2->topRight, regions);
        findDifferentRegionsRecursive(node1->bottomLeft, node2->bottomLeft, regions);
        findDifferentRegionsRecursive(node1->bottomRight, node2->bottomRight, regions);
    } else {
        // One is leaf and other is not, add the entire region
        regions.push_back(node1->region);
    }
}
