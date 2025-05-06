#include "ImageProcessor.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <iostream>

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
    if (image.empty() || image.cols <= 0 || image.rows <= 0) {
        throw std::runtime_error("Invalid image dimensions for grayscale conversion");
    }

    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    return grayImage;
}

// Processes a specific region (ROI) of an image
cv::Mat processImage(const cv::Mat& image, const cv::Rect& roi) {
    // Ensure the image is valid
    if (image.empty() || image.cols <= 0 || image.rows <= 0) {
        throw std::runtime_error("Invalid image dimensions for processing");
    }

    // Validate ROI dimensions
    if (roi.x < 0 || roi.y < 0 || roi.width <= 0 || roi.height <= 0 ||
        roi.x + roi.width > image.cols || roi.y + roi.height > image.rows) {
        throw std::invalid_argument("Invalid ROI dimensions");
    }

    // Extract ROI
    cv::Mat roiImage = image(roi);
    return roiImage;
}
