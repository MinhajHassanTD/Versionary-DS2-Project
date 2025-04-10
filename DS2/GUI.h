#ifndef GUI_H
#define GUI_H

#include <string>

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
};

#endif // GUI_H