#include "ImageProcessor.h"
#include <sstream>
#include <iomanip>
#include <functional>

ImageProcessor::ImageProcessor() : numThreads_(std::thread::hardware_concurrency()) {}

bool ImageProcessor::loadImage(const std::string& filePath) {
    image_ = cv::imread(filePath, cv::IMREAD_UNCHANGED);
    return !image_.empty();
}

bool ImageProcessor::saveImage(const std::string& filePath) {
    return cv::imwrite(filePath, image_);
}

void ImageProcessor::convertToGrayscale() {
    if (image_.channels() > 1) {
        cv::cvtColor(image_, image_, cv::COLOR_BGR2GRAY);
    }
}

cv::Mat ImageProcessor::getImage() const {
    return image_;
}

void ImageProcessor::setNumThreads(unsigned int numThreads) {
    numThreads_ = numThreads == 0 ? std::thread::hardware_concurrency() : numThreads;
}

unsigned int ImageProcessor::getNumThreads() const {
    return numThreads_;
}

Quadtree ImageProcessor::createQuadtree(int maxDepth, int minSize, double threshold, bool useParallel) const {
    // Generate cache key
    std::stringstream ss;
    ss << "quadtree_" << maxDepth << "_" << minSize << "_" << threshold << "_" << useParallel;
    std::string cacheKey = generateCacheKey("createQuadtree", ss.str());

    // Check cache
    auto [found, cachedResult] = getCachedResult(cacheKey);
    if (found) {
        // Use cached quadtree
        return Quadtree(cachedResult, maxDepth, minSize, threshold);
    }

    // Convert to grayscale for processing if not already
    cv::Mat grayImage;
    if (image_.channels() > 1) {
        cv::cvtColor(image_, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = image_.clone();
    }

    // Cache the grayscale image
    cacheResult(cacheKey + "_gray", grayImage);

    if (useParallel && numThreads_ > 1) {
        // Create quadtree with parallel processing
        cv::Rect rootRegion(0, 0, grayImage.cols, grayImage.rows);
        auto rootNode = processRegionParallel(rootRegion, 0, maxDepth, minSize, threshold);

        // Create quadtree from root node
        Quadtree quadtree(grayImage, rootNode, maxDepth, minSize, threshold);
        return quadtree;
    } else {
        // Create quadtree without parallel processing
        return Quadtree(grayImage, maxDepth, minSize, threshold);
    }
}

MerkleTree ImageProcessor::createMerkleTree(int maxDepth, int minSize, double threshold, bool useParallel) const {
    // Generate cache key
    std::stringstream ss;
    ss << "merkletree_" << maxDepth << "_" << minSize << "_" << threshold << "_" << useParallel;
    std::string cacheKey = generateCacheKey("createMerkleTree", ss.str());

    // Check cache
    auto [found, cachedResult] = getCachedResult(cacheKey);
    if (found && !cachedResult.empty()) {
        // Deserialize MerkleTree from cached result
        // For simplicity, we'll just recreate it
    }

    // Create Quadtree first
    Quadtree quadtree = createQuadtree(maxDepth, minSize, threshold, useParallel);

    // Get leaf hashes from Quadtree
    std::vector<std::string> leafHashes = quadtree.getLeafHashes();

    // Create Merkle Tree from leaf hashes
    return MerkleTree(leafHashes);
}

cv::Mat ImageProcessor::compareImages(const cv::Mat& other, int maxDepth, int minSize, double threshold, bool useParallel) const {
    // Generate cache key
    std::stringstream ss;
    ss << "compare_" << maxDepth << "_" << minSize << "_" << threshold << "_" << useParallel;
    std::string cacheKey = generateCacheKey("compareImages", ss.str());

    // Check cache
    auto [found, cachedResult] = getCachedResult(cacheKey);
    if (found) {
        return cachedResult;
    }

    // Convert both images to grayscale for processing
    cv::Mat grayImage1, grayImage2;

    if (image_.channels() > 1) {
        cv::cvtColor(image_, grayImage1, cv::COLOR_BGR2GRAY);
    } else {
        grayImage1 = image_.clone();
    }

    if (other.channels() > 1) {
        cv::cvtColor(other, grayImage2, cv::COLOR_BGR2GRAY);
    } else {
        grayImage2 = other.clone();
    }

    // Resize second image to match first if needed
    if (grayImage1.size() != grayImage2.size()) {
        cv::resize(grayImage2, grayImage2, grayImage1.size());
    }

    // Create Quadtrees for both images
    Quadtree quadtree1 = createQuadtree(maxDepth, minSize, threshold, useParallel);

    // Create a temporary ImageProcessor for the second image
    ImageProcessor tempProcessor;
    tempProcessor.setNumThreads(numThreads_);
    tempProcessor.loadImage(other);
    Quadtree quadtree2 = tempProcessor.createQuadtree(maxDepth, minSize, threshold, useParallel);

    // Find different regions
    std::vector<cv::Rect> differentRegions = quadtree1.findDifferentRegions(quadtree2);

    // Create a visualization
    cv::Mat result;
    cv::cvtColor(grayImage1, result, cv::COLOR_GRAY2BGR);

    // Highlight different regions
    result = highlightRegions(result, differentRegions, cv::Scalar(0, 0, 255));

    // Cache the result
    cacheResult(cacheKey, result);

    return result;
}

cv::Mat ImageProcessor::visualizeQuadtree(int maxDepth, int minSize, double threshold, bool useParallel) const {
    // Generate cache key
    std::stringstream ss;
    ss << "visualize_" << maxDepth << "_" << minSize << "_" << threshold << "_" << useParallel;
    std::string cacheKey = generateCacheKey("visualizeQuadtree", ss.str());

    // Check cache
    auto [found, cachedResult] = getCachedResult(cacheKey);
    if (found) {
        return cachedResult;
    }

    // Create Quadtree
    Quadtree quadtree = createQuadtree(maxDepth, minSize, threshold, useParallel);

    // Visualize Quadtree structure
    cv::Mat result = quadtree.visualize(image_);

    // Cache the result
    cacheResult(cacheKey, result);

    return result;
}

cv::Mat ImageProcessor::highlightRegions(const cv::Mat& image, const std::vector<cv::Rect>& regions, const cv::Scalar& color) const {
    cv::Mat result = image.clone();

    // Convert to color if grayscale
    if (result.channels() == 1) {
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }

    // Draw rectangles around different regions
    for (const auto& region : regions) {
        cv::rectangle(result, region, color, 2);
    }

    return result;
}

std::shared_ptr<Quadtree::Node> ImageProcessor::processRegionParallel(const cv::Rect& region, int depth, int maxDepth, int minSize, double threshold) const {
    auto node = std::make_shared<Quadtree::Node>(region);

    // Stop if maximum depth reached or region is too small
    if (depth >= maxDepth ||
        region.width <= minSize ||
        region.height <= minSize ||
        isHomogeneous(region)) {

        // Compute hash for leaf node
        node->hash = computeHash(region);
        node->isLeaf = true;
        return node;
    }

    // Split region into four quadrants
    int halfWidth = region.width / 2;
    int halfHeight = region.height / 2;

    cv::Rect topLeftRegion(region.x, region.y, halfWidth, halfHeight);
    cv::Rect topRightRegion(region.x + halfWidth, region.y,
                           region.width - halfWidth, halfHeight);
    cv::Rect bottomLeftRegion(region.x, region.y + halfHeight,
                             halfWidth, region.height - halfHeight);
    cv::Rect bottomRightRegion(region.x + halfWidth, region.y + halfHeight,
                              region.width - halfWidth, region.height - halfHeight);

    // Process child nodes in parallel if not at a deep level
    if (depth < 2 && numThreads_ > 1) {
        // Use async to process in parallel
        auto futureTopLeft = std::async(std::launch::async,
            [this, &topLeftRegion, depth, maxDepth, minSize, threshold]() {
                return processRegionParallel(topLeftRegion, depth + 1, maxDepth, minSize, threshold);
            });

        auto futureTopRight = std::async(std::launch::async,
            [this, &topRightRegion, depth, maxDepth, minSize, threshold]() {
                return processRegionParallel(topRightRegion, depth + 1, maxDepth, minSize, threshold);
            });

        auto futureBottomLeft = std::async(std::launch::async,
            [this, &bottomLeftRegion, depth, maxDepth, minSize, threshold]() {
                return processRegionParallel(bottomLeftRegion, depth + 1, maxDepth, minSize, threshold);
            });

        auto futureBottomRight = std::async(std::launch::async,
            [this, &bottomRightRegion, depth, maxDepth, minSize, threshold]() {
                return processRegionParallel(bottomRightRegion, depth + 1, maxDepth, minSize, threshold);
            });

        // Get results
        node->topLeft = futureTopLeft.get();
        node->topRight = futureTopRight.get();
        node->bottomLeft = futureBottomLeft.get();
        node->bottomRight = futureBottomRight.get();
    } else {
        // Process sequentially
        node->topLeft = processRegionParallel(topLeftRegion, depth + 1, maxDepth, minSize, threshold);
        node->topRight = processRegionParallel(topRightRegion, depth + 1, maxDepth, minSize, threshold);
        node->bottomLeft = processRegionParallel(bottomLeftRegion, depth + 1, maxDepth, minSize, threshold);
        node->bottomRight = processRegionParallel(bottomRightRegion, depth + 1, maxDepth, minSize, threshold);
    }

    // Mark as non-leaf
    node->isLeaf = false;

    // Compute hash for internal node by combining child hashes
    std::vector<uchar> combinedHash;
    combinedHash.insert(combinedHash.end(), node->topLeft->hash.begin(), node->topLeft->hash.end());
    combinedHash.insert(combinedHash.end(), node->topRight->hash.begin(), node->topRight->hash.end());
    combinedHash.insert(combinedHash.end(), node->bottomLeft->hash.begin(), node->bottomLeft->hash.end());
    combinedHash.insert(combinedHash.end(), node->bottomRight->hash.begin(), node->bottomRight->hash.end());

    node->hash = sha256_qt(combinedHash);

    return node;
}

std::pair<bool, cv::Mat> ImageProcessor::getCachedResult(const std::string& key) const {
    std::lock_guard<std::mutex> lock(cacheMutex_);

    auto it = imageCache_.find(key);
    if (it != imageCache_.end()) {
        return {true, it->second};
    }

    return {false, cv::Mat()};
}

void ImageProcessor::cacheResult(const std::string& key, const cv::Mat& result) const {
    std::lock_guard<std::mutex> lock(cacheMutex_);

    // Limit cache size to prevent memory issues
    if (imageCache_.size() > 100) {
        // Simple strategy: clear the entire cache
        imageCache_.clear();
    }

    imageCache_[key] = result.clone();
}

std::string ImageProcessor::generateCacheKey(const std::string& operation, const std::string& params) const {
    // Create a hash of the image to include in the cache key
    cv::Mat smallImage;
    cv::resize(image_, smallImage, cv::Size(32, 32));
    std::vector<uchar> buffer;
    cv::imencode(".jpg", smallImage, buffer);
    std::string imageHash = sha256_qt(buffer);

    return operation + "_" + imageHash.substr(0, 8) + "_" + params;
}

bool ImageProcessor::isHomogeneous(const cv::Rect& region) const {
    cv::Mat roi = image_(region);
    cv::Scalar mean, stddev;
    cv::meanStdDev(roi, mean, stddev);

    // Check if standard deviation is below threshold
    double maxStdDev = 0;
    for (int i = 0; i < stddev.channels; i++) {
        maxStdDev = std::max(maxStdDev, stddev[i]);
    }

    return maxStdDev < 10.0; // Using a fixed threshold for simplicity
}

std::string ImageProcessor::computeHash(const cv::Rect& region) const {
    cv::Mat roi = image_(region);
    std::vector<uchar> buffer;
    cv::imencode(".png", roi, buffer);
    return sha256_qt(buffer);
}
