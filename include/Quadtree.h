#ifndef QUADTREE_H
#define QUADTREE_H

#include <memory>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

/**
 * @brief Quadtree class for image chunking
 *
 * This class implements a Quadtree data structure for breaking down
 * images into smaller chunks. It recursively divides the image into
 * four quadrants until a specified depth or homogeneity criterion is met.
 */
class Quadtree {
public:
    /**
     * @brief Node structure for the Quadtree
     */
    struct Node {
        cv::Rect region;
        std::string hash;
        bool isLeaf;
        std::shared_ptr<Node> topLeft;
        std::shared_ptr<Node> topRight;
        std::shared_ptr<Node> bottomLeft;
        std::shared_ptr<Node> bottomRight;

        Node(const cv::Rect& region)
            : region(region), isLeaf(true),
              topLeft(nullptr), topRight(nullptr),
              bottomLeft(nullptr), bottomRight(nullptr) {}
    };

    /**
     * @brief Construct a new Quadtree from an image
     *
     * @param image OpenCV image to process
     * @param maxDepth Maximum depth of the tree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     */
    Quadtree(const cv::Mat& image, int maxDepth = 8, int minSize = 8, double threshold = 10.0);

    /**
     * @brief Construct a new Quadtree from an image with a pre-built root node
     *
     * @param image OpenCV image to process
     * @param rootNode Pre-built root node
     * @param maxDepth Maximum depth of the tree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     */
    Quadtree(const cv::Mat& image, std::shared_ptr<Node> rootNode, int maxDepth = 8, int minSize = 8, double threshold = 10.0);

    /**
     * @brief Get the root node of the Quadtree
     *
     * @return std::shared_ptr<Node> Root node
     */
    std::shared_ptr<Node> getRoot() const;

    /**
     * @brief Get all leaf nodes of the Quadtree
     *
     * @return std::vector<std::shared_ptr<Node>> Vector of leaf nodes
     */
    std::vector<std::shared_ptr<Node>> getLeafNodes() const;

    /**
     * @brief Get all leaf hashes in a consistent order
     *
     * @return std::vector<std::string> Vector of leaf hashes
     */
    std::vector<std::string> getLeafHashes() const;

    /**
     * @brief Visualize the Quadtree structure on an image
     *
     * @param image Image to draw on
     * @return cv::Mat Image with Quadtree visualization
     */
    cv::Mat visualize(const cv::Mat& image) const;

    /**
     * @brief Compare this Quadtree with another to find different regions
     *
     * @param other Another Quadtree to compare with
     * @return std::vector<cv::Rect> Regions that differ between trees
     */
    std::vector<cv::Rect> findDifferentRegions(const Quadtree& other) const;

private:
    std::shared_ptr<Node> root_;
    cv::Mat image_;
    int maxDepth_;
    int minSize_;
    double threshold_;
    std::vector<std::shared_ptr<Node>> leafNodes_;

    /**
     * @brief Build the Quadtree recursively
     *
     * @param node Current node to process
     * @param depth Current depth in the tree
     */
    void buildTree(std::shared_ptr<Node> node, int depth);

    /**
     * @brief Check if a region is homogeneous
     *
     * @param region Region to check
     * @return bool True if the region is homogeneous
     */
    bool isHomogeneous(const cv::Rect& region) const;

    /**
     * @brief Compute hash for a region
     *
     * @param region Region to hash
     * @return std::string Hash of the region
     */
    std::string computeHash(const cv::Rect& region) const;

    /**
     * @brief Collect all leaf nodes recursively
     *
     * @param node Current node to process
     * @param leafNodes Vector to store leaf nodes
     */
    void collectLeafNodes(const std::shared_ptr<Node>& node,
                         std::vector<std::shared_ptr<Node>>& leafNodes) const;

    /**
     * @brief Draw the Quadtree structure recursively
     *
     * @param image Image to draw on
     * @param node Current node to draw
     */
    void drawNode(cv::Mat& image, const std::shared_ptr<Node>& node) const;

    /**
     * @brief Find different regions recursively
     *
     * @param node1 Node from first tree
     * @param node2 Node from second tree
     * @param regions Vector to store different regions
     */
    void findDifferentRegionsRecursive(const std::shared_ptr<Node>& node1,
                                     const std::shared_ptr<Node>& node2,
                                     std::vector<cv::Rect>& regions) const;
};

#endif // QUADTREE_H
