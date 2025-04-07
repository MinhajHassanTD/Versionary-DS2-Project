#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QPixmap>
#include <QImage>
#include <opencv2/opencv.hpp>
#include "VersionControl.h"

/**
 * @brief GUI class for graphical user interface
 *
 * This class provides a graphical user interface for interacting
 * with the version control system. It includes functionality for
 * adding, committing, comparing, and rolling back versions.
 */
class GUI : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Construct a new GUI
     *
     * @param versionControl Version control system to use
     * @param parent Parent widget
     */
    GUI(VersionControl& versionControl, QWidget* parent = nullptr);

    /**
     * @brief Destroy the GUI
     */
    ~GUI();

private slots:
    /**
     * @brief Handle open repository action
     */
    void openRepository();

    /**
     * @brief Handle create repository action
     */
    void createRepository();

    /**
     * @brief Handle add image action
     */
    void addImage();

    /**
     * @brief Handle commit image action
     */
    void commitImage();

    /**
     * @brief Handle version selection
     *
     * @param item Selected item
     */
    void versionSelected(QListWidgetItem* item);

    /**
     * @brief Handle compare versions action
     */
    void compareVersions();

    /**
     * @brief Handle rollback action
     */
    void rollbackToVersion();

    /**
     * @brief Handle visualize Quadtree action
     */
    void visualizeQuadtree();

    /**
     * @brief Handle about action
     */
    void about();

private:
    VersionControl& versionControl_;

    // UI components
    QListWidget* versionList_;
    QLabel* imageLabel_;
    QLabel* diffImageLabel_;
    QLineEdit* commitMessageEdit_;
    QPushButton* addButton_;
    QPushButton* commitButton_;
    QPushButton* compareButton_;
    QPushButton* rollbackButton_;
    QPushButton* visualizeButton_;
    QComboBox* compareVersionCombo_;
    QScrollArea* imageScrollArea_;
    QScrollArea* diffImageScrollArea_;
    QStatusBar* statusBar_;
    QCheckBox* encryptCheckBox_;
    QCheckBox* signCheckBox_;

    // Current state
    std::string currentVersionId_;
    std::string selectedVersionId_;

    /**
     * @brief Create the main UI
     */
    void createUI();

    /**
     * @brief Create the menu bar
     */
    void createMenuBar();

    /**
     * @brief Create the tool bar
     */
    void createToolBar();

    /**
     * @brief Update the version list
     */
    void updateVersionList();

    /**
     * @brief Display an image
     *
     * @param image Image to display
     * @param label Label to display on
     */
    void displayImage(const cv::Mat& image, QLabel* label);

    /**
     * @brief Convert OpenCV image to Qt image
     *
     * @param image OpenCV image
     * @return QImage Qt image
     */
    QImage matToQImage(const cv::Mat& image);

    /**
     * @brief Show error message
     *
     * @param message Error message
     */
    void showError(const QString& message);

    /**
     * @brief Show success message
     *
     * @param message Success message
     */
    void showSuccess(const QString& message);
};

#endif // GUI_H
