#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

class Quadtree {
private:
    static const int MIN_SIZE = 8;
    
    cv::Rect region;
    std::string hash;
    bool isLeaf;
    std::shared_ptr<Quadtree> children[4]; // TL, TR, BL, BR
    
    bool isHomogeneous(const cv::Mat& image, const cv::Rect& rect, int threshold);
    std::string hashNodeData(const cv::Scalar& avgColor);
    
public:
    Quadtree() : isLeaf(false) {}
    
    void buildFromImage(const cv::Mat& image, int threshold = 10);
    std::vector<cv::Rect> compareWith(const Quadtree& other);
    cv::Mat visualizeDiff(const cv::Mat& image, const std::vector<cv::Rect>& differences);
};