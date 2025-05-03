#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

class Utils {
public:
    // File operations
    static bool fileExists(const std::string& filePath);
    static std::string readFile(const std::string& filePath);
    static void writeFile(const std::string& filePath, const std::string& content);

    // String operations
    static std::vector<std::string> splitString(const std::string& input, char delimiter);
    
    // Perceptual hashing
    static std::string computePerceptualHash(const cv::Mat& image);
    static int hammingDistance(const std::string& hash1, const std::string& hash2);
    
private:
    // Helper methods for perceptual hashing
    static cv::Mat preprocessForHash(const cv::Mat& image);
};

#endif // UTILS_H
