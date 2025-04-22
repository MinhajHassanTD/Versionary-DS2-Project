#ifndef IMAGECOMPARER_H
#define IMAGECOMPARER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>
#include "Quadtree.h"

class ImageComparer {
public:
    static cv::Mat compareImages(const cv::Mat& image1, const cv::Mat& image2);
    static void visualizeDifferences(const cv::Mat& differences, const std::string& outputPath);
    
    // Enhanced methods using Quadtree and Merkle Tree
    static std::vector<cv::Rect> compareWithStructures(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize);
    static void highlightDifferences(const cv::Mat& image, const std::vector<cv::Rect>& diffRegions, const std::string& outputPath);
    
private:
    // Helper methods
    static void collectHashesWithRegions(std::shared_ptr<QuadtreeNode> node, 
                                        std::vector<std::string>& hashes, 
                                        std::map<std::string, cv::Rect>& hashToRegion);
    static std::string hashImageChunk(const cv::Mat& chunk);
};

#endif // IMAGECOMPARER_H
//new
