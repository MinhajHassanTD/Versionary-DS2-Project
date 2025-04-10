#ifndef QUADTREE_H
#define QUADTREE_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

// Forward declaration of QuadtreeNode class
class QuadtreeNode {
public:
    QuadtreeNode(const cv::Rect& region, const cv::Mat& image);
    bool isLeaf() const;
    void subdivide();

    cv::Rect region;
    cv::Mat chunk;
    std::shared_ptr<QuadtreeNode> topLeft;
    std::shared_ptr<QuadtreeNode> topRight;
    std::shared_ptr<QuadtreeNode> bottomLeft;
    std::shared_ptr<QuadtreeNode> bottomRight;
};

class Quadtree {
public:
    Quadtree(const cv::Mat& image, int minSize = 8);
    Quadtree(int minSize = 8);
    std::shared_ptr<QuadtreeNode> getRoot() const;
    std::vector<cv::Mat> divideImage(const cv::Mat& image);
    cv::Mat reconstructImage(const std::vector<cv::Mat>& chunks, int originalWidth, int originalHeight);

private:
    void buildTree(std::shared_ptr<QuadtreeNode> node, int minSize);
    void divideRecursively(const cv::Mat& image, std::vector<cv::Mat>& chunks, int x, int y, int width, int height);
    bool isHomogeneous(const cv::Mat& image, int x, int y, int width, int height);

    int minSize;
    int minChunkSize;
    std::shared_ptr<QuadtreeNode> root;
};

#endif // QUADTREE_H
