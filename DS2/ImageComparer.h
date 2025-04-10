#ifndef IMAGECOMPARER_H
#define IMAGECOMPARER_H

#include <opencv2/opencv.hpp>
#include <string>
#include "Quadtree.h"

class ImageComparer {
public:
    static cv::Mat compareImages(const cv::Mat& image1, const cv::Mat& image2);
    static void visualizeDifferences(const cv::Mat& differences, const std::string& outputPath);
    static cv::Mat compareWithQuadtree(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize = 8);
};

#endif // IMAGECOMPARER_H
