#include "ImageProcessor.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

// Reads an image from the given file path
cv::Mat ImageProcessor::readImage(const std::string& filePath) {
    cv::Mat image = cv::imread(filePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error("Failed to read image from: " + filePath);
    }
    return image;
}

// Converts a given image to grayscale
cv::Mat ImageProcessor::convertToGrayscale(const cv::Mat& image) {
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    return grayImage;
}

// Saves an image to the given file path
void ImageProcessor::saveImage(const cv::Mat& image, const std::string& filePath) {
    if (!cv::imwrite(filePath, image)) {
        throw std::runtime_error("Failed to save image to: " + filePath);
    }
}

// Resizes an image to the specified dimensions
cv::Mat ImageProcessor::resizeImage(const cv::Mat& image, int width, int height) {
    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(width, height));
    return resizedImage;
}

// Converts an image to a string representation (for hashing or storage)
std::string ImageProcessor::imageToString(const cv::Mat& image) {
    if (image.empty()) {
        throw std::runtime_error("Cannot convert empty image to string");
    }
    
    // Create a header with image metadata
    std::string header = std::to_string(image.rows) + "," + 
                         std::to_string(image.cols) + "," + 
                         std::to_string(image.type()) + ";";
    
    // Convert image data to string
    std::string data(reinterpret_cast<char*>(image.data), image.total() * image.elemSize());
    
    return header + data;
}

// Converts a string representation back to an image
cv::Mat ImageProcessor::stringToImage(const std::string& str, int rows, int cols, int type) {
    // Extract image data from the string
    size_t dataSize = rows * cols * (type % 8 + 1);
    
    if (str.size() < dataSize) {
        throw std::runtime_error("String is too short to contain the specified image");
    }
    
    // Create a new image and copy the data
    cv::Mat image(rows, cols, type);
    std::memcpy(image.data, str.data(), dataSize);
    
    return image;
}
