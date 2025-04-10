#include "ImageComparer.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <stdexcept>

// Compares two images and returns a difference map
cv::Mat ImageComparer::compareImages(const cv::Mat& image1, const cv::Mat& image2) {
    if (image1.size() != image2.size() || image1.type() != image2.type()) {
        throw std::runtime_error("Images must have the same size and type for comparison.");
    }

    cv::Mat diff, diffGray, thresholded;
    cv::absdiff(image1, image2, diff); // Compute absolute difference
    cv::cvtColor(diff, diffGray, cv::COLOR_BGR2GRAY); // Convert to grayscale
    cv::threshold(diffGray, thresholded, 50, 255, cv::THRESH_BINARY); // Threshold differences

    return thresholded;
}

// Visualizes the differences by saving an output image
void ImageComparer::visualizeDifferences(const cv::Mat& differences, const std::string& outputPath) {
    cv::Mat visualized;
    cv::applyColorMap(differences, visualized, cv::COLORMAP_JET); // Apply color map for better visualization
    if (!cv::imwrite(outputPath, visualized)) {
        throw std::runtime_error("Failed to save visualized differences to: " + outputPath);
    }
}

// Compares two images using Quadtree for more efficient comparison
cv::Mat ImageComparer::compareWithQuadtree(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize) {
    if (image1.size() != image2.size() || image1.type() != image2.type()) {
        throw std::runtime_error("Images must have the same size and type for comparison.");
    }
    
    // Convert images to grayscale if they're not already
    cv::Mat gray1, gray2;
    if (image1.channels() > 1) {
        cv::cvtColor(image1, gray1, cv::COLOR_BGR2GRAY);
        cv::cvtColor(image2, gray2, cv::COLOR_BGR2GRAY);
    } else {
        gray1 = image1.clone();
        gray2 = image2.clone();
    }
    
    // Create a Quadtree and divide both images
    Quadtree quadtree(minChunkSize);
    std::vector<cv::Mat> chunks1 = quadtree.divideImage(gray1);
    std::vector<cv::Mat> chunks2 = quadtree.divideImage(gray2);
    
    // Create a result image to store differences
    cv::Mat result = cv::Mat::zeros(gray1.size(), CV_8UC1);
    
    // Compare corresponding chunks
    // This is a simplified approach; a more sophisticated implementation would need to match chunks
    size_t minChunks = std::min(chunks1.size(), chunks2.size());
    for (size_t i = 0; i < minChunks; i++) {
        cv::Mat chunkDiff;
        cv::absdiff(chunks1[i], chunks2[i], chunkDiff);
        
        // If there's a significant difference in this chunk, mark it in the result
        double maxVal;
        cv::minMaxLoc(chunkDiff, nullptr, &maxVal);
        
        if (maxVal > 30) { // Threshold can be adjusted
            // Mark this chunk as different in the result
            // This is a simplified approach; you would need to track chunk positions
            // Around line 67, replace:
            cv::rectangle(result, cv::Rect(0, 0, chunks1[i].cols, chunks1[i].rows), cv::Scalar(255), -1);
            
            // With something that tracks positions:
            // You need to implement a way to track chunk positions
        }
    }
    
    return result;
}
