#ifndef IMAGECOMPARER_H
#define IMAGECOMPARER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <map>
#include "Quadtree.h"

class ImageComparer {
public:
    static cv::Mat compareImages(const cv::Mat& image1, const cv::Mat& image2, int sensitivity = 65);
    static void visualizeDifferences(const cv::Mat& differences, const std::string& outputPath);
    
    static std::vector<cv::Rect> compareWithStructures(const cv::Mat& image1, const cv::Mat& image2, int minChunkSize, int sensitivity = 10);
    static void highlightDifferences(const cv::Mat& image, const std::vector<cv::Rect>& diffRegions, const std::string& outputPath);
private:
    // Helper methods
    static void collectHashesWithRegions(std::shared_ptr<QuadtreeNode> node, 
                                        std::vector<std::string>& hashes, 
                                        std::map<std::string, cv::Rect>& hashToRegion);
    static std::string hashImageChunk(const cv::Mat& chunk);
    static bool areHashesSimilar(const std::string& hash1, const std::string& hash2, int threshold);
};

#endif // IMAGECOMPARER_H
