#include "Utils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <bitset>
#include <opencv2/imgproc.hpp>

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

// Perceptual Hash implementation (pHash)
std::string Utils::computePerceptualHash(const cv::Mat& image) {
    // 1. Convert to grayscale and resize to 32x32
    cv::Mat resized = preprocessForHash(image);
    
    // 2. Apply DCT (Discrete Cosine Transform)
    cv::Mat dctImage;
    cv::dct(resized, dctImage);
    
    // 3. Extract the top-left 8x8 corner (low frequencies)
    cv::Mat dctLowFreq = dctImage(cv::Rect(0, 0, 8, 8));
    
    // 4. Calculate the mean of the 8x8 low frequencies (excluding DC component)
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (!(i == 0 && j == 0)) { // Skip the DC component (0,0)
                sum += dctLowFreq.at<float>(i, j);
                count++;
            }
        }
    }
    double mean = sum / count;
    
    // 5. Generate a 64-bit hash based on whether each value is above the mean
    std::string hash;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (dctLowFreq.at<float>(i, j) > mean) {
                hash += "1";
            } else {
                hash += "0";
            }
        }
    }
    
    return hash;
}

// Helper for preprocessing images before hashing
cv::Mat Utils::preprocessForHash(const cv::Mat& image) {
    cv::Mat grayscale, resized;
    
    // Convert to grayscale if necessary
    if (image.channels() == 3 || image.channels() == 4) {
        cv::cvtColor(image, grayscale, cv::COLOR_BGR2GRAY);
    } else {
        grayscale = image.clone();
    }
    
    // Resize to 32x32 and convert to float for DCT
    cv::resize(grayscale, resized, cv::Size(32, 32), 0, 0, cv::INTER_AREA);
    resized.convertTo(resized, CV_32F);
    
    return resized;
}

// Calculate Hamming distance between two hashes
int Utils::hammingDistance(const std::string& hash1, const std::string& hash2) {
    if (hash1.length() != hash2.length()) {
        return -1; // Error: hashes should be the same length
    }
    
    int distance = 0;
    for (size_t i = 0; i < hash1.length(); i++) {
        if (hash1[i] != hash2[i]) {
            distance++;
        }
    }
    
    return distance;
}
