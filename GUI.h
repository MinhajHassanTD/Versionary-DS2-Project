#ifndef GUI_H
#define GUI_H

#include <string>
#include <vector>
#include "Quadtree.h"

class GUI {
public:
    void initialize();
    void displayMainMenu();
    void handleAddImage();
    void handleCompareImages();
    void handleViewVersion();
    void handleExit();

private:
    void showError(const std::string& message);
    
    // Helper functions for hashing
    void collectLeafHashes(std::shared_ptr<QuadtreeNode> node, std::vector<std::string>& hashes);
    std::string hashImageChunk(const cv::Mat& chunk);
};

#endif // GUI_H
