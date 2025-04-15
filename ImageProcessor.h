#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <string>
#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    static cv::Mat readImage(const std::string& filePath);
    static cv::Mat convertToGrayscale(const cv::Mat& image);
};

#endif // IMAGEPROCESSOR_H
