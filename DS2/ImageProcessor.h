#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <string>
#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    static cv::Mat readImage(const std::string& filePath);
    static cv::Mat convertToGrayscale(const cv::Mat& image);
    static void saveImage(const cv::Mat& image, const std::string& filePath);
    static cv::Mat resizeImage(const cv::Mat& image, int width, int height);
    static std::string imageToString(const cv::Mat& image);
    static cv::Mat stringToImage(const std::string& str, int rows, int cols, int type);
};

#endif // IMAGEPROCESSOR_H
