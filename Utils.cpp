#include "Utils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <bitset>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

// Checks if a file exists
bool Utils::fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

// Reads the content of a file
std::string Utils::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Writes content to a file
void Utils::writeFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }
    
    file << content;
}

// Splits a string by a delimiter
std::vector<std::string> Utils::splitString(const std::string& input, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// Improved perceptual hash function with better error handling
std::string Utils::computePerceptualHash(const cv::Mat& image) {
    try {
        // Ensure we have a valid image
        if (image.empty()) {
            throw std::runtime_error("Empty image provided for perceptual hashing");
        }

        // 1. Convert to grayscale and resize to 32x32
        cv::Mat resized = preprocessForHash(image);
        
        // 2. Apply DCT (Discrete Cosine Transform)
        cv::Mat dctImage;
        cv::dct(resized, dctImage);
        
        // 3. Extract the top-left 8x8 corner (low frequencies)
        cv::Mat dctLowFreq = dctImage(cv::Rect(0, 0, 8, 8));
        
        // 4. Calculate the median of the 8x8 low frequencies (excluding DC component)
        // (using median instead of mean provides better resistance to outliers)
        std::vector<float> coefficients;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (!(i == 0 && j == 0)) { // Skip the DC component (0,0)
                    coefficients.push_back(dctLowFreq.at<float>(i, j));
                }
            }
        }
        
        // Sort to find median
        std::sort(coefficients.begin(), coefficients.end());
        double median = coefficients.size() % 2 == 0 
            ? (coefficients[coefficients.size()/2 - 1] + coefficients[coefficients.size()/2]) / 2
            : coefficients[coefficients.size()/2];
        
        // 5. Generate a 64-bit hash based on whether each value is above the median
        std::string hash;
        hash.reserve(64); // Pre-allocate for efficiency
        
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                hash += (dctLowFreq.at<float>(i, j) > median) ? "1" : "0";
            }
        }
        
        return hash;
    }
    catch (const cv::Exception& e) {
        throw std::runtime_error("OpenCV error during perceptual hashing: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error during perceptual hashing: " + std::string(e.what()));
    }
}

// Improved preprocessForHash function with more robust handling
cv::Mat Utils::preprocessForHash(const cv::Mat& image) {
    try {
        cv::Mat grayscale, resized;
        
        // Convert to grayscale if necessary
        if (image.channels() == 3 || image.channels() == 4) {
            cv::cvtColor(image, grayscale, cv::COLOR_BGR2GRAY);
        } else if (image.channels() == 1) {
            grayscale = image.clone();
        } else {
            throw std::runtime_error("Unsupported image format with " + 
                                    std::to_string(image.channels()) + " channels");
        }
        
        // Apply light blur to reduce noise and compression artifacts
        cv::GaussianBlur(grayscale, grayscale, cv::Size(3, 3), 0);
        
        // Resize to 32x32 and convert to float for DCT
        cv::resize(grayscale, resized, cv::Size(32, 32), 0, 0, cv::INTER_AREA);
        resized.convertTo(resized, CV_32F);
        
        return resized;
    }
    catch (const cv::Exception& e) {
        throw std::runtime_error("OpenCV error during image preprocessing: " + std::string(e.what()));
    }
}

// Optimize hammingDistance calculation for better performance
int Utils::hammingDistance(const std::string& hash1, const std::string& hash2) {
    if (hash1.length() != hash2.length()) {
        return -1; // Error: hashes should be the same length
    }
    
    // Use direct comparison for exact equality - very fast path
    if (hash1 == hash2) {
        return 0;
    }
    
    // We'll weight differences in low frequency components more heavily
    double distance = 0;
    
    // Unroll the loop a bit for better performance
    const size_t len = hash1.length();
    size_t i = 0;
    
    // Process 8 elements at once for better CPU utilization
    for (; i + 8 <= len; i += 8) {
        for (size_t j = 0; j < 8; j++) {
            if (hash1[i+j] != hash2[i+j]) {
                // Calculate position in the 8x8 grid
                int row = (i+j) / 8;
                int col = (i+j) % 8;
                
                // Compute weight based on distance from top-left (DC) component
                // Closer to DC = more important = higher weight for differences
                double weight = 1.0 / (1.0 + sqrt(row*row + col*col));
                distance += weight;
            }
        }
    }
    
    // Process remaining elements
    for (; i < len; i++) {
        if (hash1[i] != hash2[i]) {
            int row = i / 8;
            int col = i % 8;
            double weight = 1.0 / (1.0 + sqrt(row*row + col*col));
            distance += weight;
        }
    }
    
    // Normalize to 0-64 range for compatibility with the rest of the code
    return static_cast<int>(distance * 20); // Multiplier adjusts sensitivity
}

// Create a simplified hash function for faster processing
std::string Utils::computeFastHash(const cv::Mat& image) {
    // Ensure the image is valid
    if (image.empty()) {
        throw std::runtime_error("Empty image provided for fast hashing");
    }

    // Resize to a small fixed size (faster than 32x32)
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(16, 16), 0, 0, cv::INTER_AREA);
    
    // Convert to grayscale if not already
    cv::Mat gray;
    if (resized.channels() > 1) {
        cv::cvtColor(resized, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = resized;
    }
    
    // Binarize based on median value (faster than DCT)
    double median = cv::mean(gray)[0];
    
    std::string hash;
    hash.reserve(256); // 16x16 = 256 bits
    
    for (int i = 0; i < gray.rows; i++) {
        for (int j = 0; j < gray.cols; j++) {
            hash += (gray.at<uchar>(i, j) > median) ? "1" : "0";
        }
    }
    
    return hash;
}
