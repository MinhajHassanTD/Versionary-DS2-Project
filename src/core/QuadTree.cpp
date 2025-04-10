#include "Quadtree.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

void Quadtree::buildFromImage(const cv::Mat& image, int threshold) {
    region = cv::Rect(0, 0, image.cols, image.rows);
    
    // Check if this region is homogeneous enough
    if (isHomogeneous(image, region, threshold) || 
        region.width <= MIN_SIZE || region.height <= MIN_SIZE) {
        isLeaf = true;
        
        // Store average color for this region
        cv::Scalar avgColor = cv::mean(image(region));
        hash = hashNodeData(avgColor);
    } else {
        isLeaf = false;
        
        // Subdivide into four quadrants
        int halfWidth = region.width / 2;
        int halfHeight = region.height / 2;
        
        cv::Rect quadrants[4] = {
            cv::Rect(region.x, region.y, halfWidth, halfHeight),                         // Top-left
            cv::Rect(region.x + halfWidth, region.y, region.width - halfWidth, halfHeight), // Top-right
            cv::Rect(region.x, region.y + halfHeight, halfWidth, region.height - halfHeight), // Bottom-left
            cv::Rect(region.x + halfWidth, region.y + halfHeight, 
                    region.width - halfWidth, region.height - halfHeight)  // Bottom-right
        };
        
        // Process each quadrant
        for (int i = 0; i < 4; i++) {
            if (quadrants[i].width > 0 && quadrants[i].height > 0) {
                children[i] = std::make_shared<Quadtree>();
                children[i]->region = quadrants[i];
                children[i]->buildFromImage(image, threshold);
            }
        }
    }
}

std::vector<cv::Rect> Quadtree::compareWith(const Quadtree& other) {
    std::vector<cv::Rect> differences;
    
    // If the hashes are different, check if this is a leaf node
    if (hash != other.hash) {
        if (isLeaf || other.isLeaf) {
            // This is a leaf node with different content
            differences.push_back(region);
        } else {
            // Recursively check children
            for (int i = 0; i < 4; i++) {
                if (children[i] && other.children[i]) {
                    auto childDiffs = children[i]->compareWith(*other.children[i]);
                    differences.insert(differences.end(), childDiffs.begin(), childDiffs.end());
                } else if (children[i]) {
                    differences.push_back(children[i]->region);
                } else if (other.children[i]) {
                    differences.push_back(other.children[i]->region);
                }
            }
        }
    }
    
    return differences;
}

cv::Mat Quadtree::visualizeDiff(const cv::Mat& image, const std::vector<cv::Rect>& differences) {
    cv::Mat result = image.clone();
    
    // Draw rectangles around the differences
    for (const auto& rect : differences) {
        cv::rectangle(result, rect, cv::Scalar(0, 0, 255), 2);
    }
    
    return result;
}

bool Quadtree::isHomogeneous(const cv::Mat& image, const cv::Rect& rect, int threshold) {
    cv::Mat region = image(rect);
    cv::Scalar mean, stddev;
    cv::meanStdDev(region, mean, stddev);
    
    // Check if standard deviation is below threshold for all channels
    for (int i = 0; i < image.channels(); i++) {
        if (stddev[i] > threshold) {
            return false;
        }
    }
    
    return true;
}

std::string Quadtree::hashNodeData(const cv::Scalar& avgColor) {
    // Convert average color to string
    std::stringstream ss;
    for (int i = 0; i < 4; i++) {
        ss << std::fixed << std::setprecision(2) << avgColor[i];
    }
    
    // Hash the string
    std::string data = ss.str();
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.length());
    SHA256_Final(hash, &sha256);
    
    // Convert hash to hex string
    std::stringstream hashStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hashStream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return hashStream.str();
}