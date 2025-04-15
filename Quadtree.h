#ifndef QUADTREE_H
#define QUADTREE_H

#include <opencv2/opencv.hpp>
#include <memory>

class QuadtreeNode {
public:
    QuadtreeNode(const cv::Rect& region, const cv::Mat& image);
    bool isLeaf() const;
    void subdivide();

    std::shared_ptr<QuadtreeNode> topLeft;
    std::shared_ptr<QuadtreeNode> topRight;
    std::shared_ptr<QuadtreeNode> bottomLeft;
    std::shared_ptr<QuadtreeNode> bottomRight;

    cv::Rect region;
    cv::Mat chunk;

private:
    bool isValidRegion(const cv::Rect& region) const; // Declaration of helper function
};

class Quadtree {
public:
    Quadtree(const cv::Mat& image, int minSize);
    std::shared_ptr<QuadtreeNode> getRoot() const;

private:
    void buildTree(std::shared_ptr<QuadtreeNode> node, int minSize);

    std::shared_ptr<QuadtreeNode> root;
    int minSize;
};

#endif // QUADTREE_H
