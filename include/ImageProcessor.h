#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "Quadtree.h"
#include "MerkleTree.h"

/**
 * @brief ImageProcessor class for handling image operations
 *
 * This class provides functionality for loading, saving, and processing
 * images. It includes methods for grayscale conversion, image comparison,
 * and visualization of differences.
 */
class ImageProcessor {
public:
    /**
     * @brief Construct a new Image Processor
     */
    ImageProcessor();

    /**
     * @brief Load an image from file
     *
     * @param filePath Path to the image file
     * @return bool True if loading was successful
     */
    bool loadImage(const std::string& filePath);

    /**
     * @brief Save the current image to file
     *
     * @param filePath Path to save the image
     * @return bool True if saving was successful
     */
    bool saveImage(const std::string& filePath);

    /**
     * @brief Convert the current image to grayscale
     */
    void convertToGrayscale();

    /**
     * @brief Get the current image
     *
     * @return cv::Mat Current image
     */
    cv::Mat getImage() const;

    /**
     * @brief Create a Quadtree from the current image
     *
     * @param maxDepth Maximum depth of the Quadtree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     * @param useParallel Whether to use parallel processing
     * @return Quadtree Quadtree representation of the image
     */
    Quadtree createQuadtree(int maxDepth = 8, int minSize = 8, double threshold = 10.0, bool useParallel = true) const;

    /**
     * @brief Create a Merkle Tree from the current image using Quadtree
     *
     * @param maxDepth Maximum depth of the Quadtree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     * @param useParallel Whether to use parallel processing
     * @return MerkleTree Merkle Tree representation of the image
     */
    MerkleTree createMerkleTree(int maxDepth = 8, int minSize = 8, double threshold = 10.0, bool useParallel = true) const;

    /**
     * @brief Compare two images and visualize differences
     *
     * @param other Another image to compare with
     * @param maxDepth Maximum depth of the Quadtree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     * @param useParallel Whether to use parallel processing
     * @return cv::Mat Visualization of differences
     */
    cv::Mat compareImages(const cv::Mat& other, int maxDepth = 8, int minSize = 8, double threshold = 10.0, bool useParallel = true) const;

    /**
     * @brief Visualize the Quadtree structure on the current image
     *
     * @param maxDepth Maximum depth of the Quadtree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     * @param useParallel Whether to use parallel processing
     * @return cv::Mat Image with Quadtree visualization
     */
    cv::Mat visualizeQuadtree(int maxDepth = 8, int minSize = 8, double threshold = 10.0, bool useParallel = true) const;

    /**
     * @brief Set the number of threads to use for parallel processing
     *
     * @param numThreads Number of threads (0 = auto-detect)
     */
    void setNumThreads(unsigned int numThreads = 0);

    /**
     * @brief Get the number of threads used for parallel processing
     *
     * @return unsigned int Number of threads
     */
    unsigned int getNumThreads() const;

private:
    cv::Mat image_;
    unsigned int numThreads_;
    mutable std::unordered_map<std::string, cv::Mat> imageCache_;
    mutable std::mutex cacheMutex_;

    /**
     * @brief Highlight different regions on an image
     *
     * @param image Image to highlight on
     * @param regions Regions to highlight
     * @param color Color to use for highlighting
     * @return cv::Mat Image with highlighted regions
     */
    cv::Mat highlightRegions(const cv::Mat& image, const std::vector<cv::Rect>& regions, const cv::Scalar& color) const;

    /**
     * @brief Process an image region in parallel
     *
     * @param region Region to process
     * @param depth Current depth in the quadtree
     * @param maxDepth Maximum depth of the quadtree
     * @param minSize Minimum size of a region to split
     * @param threshold Homogeneity threshold for splitting
     * @return std::shared_ptr<Quadtree::Node> Processed node
     */
    std::shared_ptr<Quadtree::Node> processRegionParallel(const cv::Rect& region, int depth, int maxDepth, int minSize, double threshold) const;

    /**
     * @brief Check if a result is cached
     *
     * @param key Cache key
     * @return std::pair<bool, cv::Mat> Pair of (found, result)
     */
    std::pair<bool, cv::Mat> getCachedResult(const std::string& key) const;

    /**
     * @brief Cache a result
     *
     * @param key Cache key
     * @param result Result to cache
     */
    void cacheResult(const std::string& key, const cv::Mat& result) const;

    /**
     * @brief Generate a cache key
     *
     * @param operation Operation name
     * @param params Additional parameters
     * @return std::string Cache key
     */
    std::string generateCacheKey(const std::string& operation, const std::string& params) const;
};

#endif // IMAGE_PROCESSOR_H
