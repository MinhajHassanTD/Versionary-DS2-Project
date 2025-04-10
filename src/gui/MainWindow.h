#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../core/VersionManager.h"
#include "../cli/VersionaryCLI.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
private:
    VersionManager versionManager;
    VersionaryCLI cli;
    
    // UI components
    QSplitter* mainSplitter;
    
    // Left panel
    QListWidget* branchList;
    QListWidget* stagingList;
    QPushButton* addButton;
    
    // Center panel
    QLabel* imageView;
    
    // Right panel
    QListWidget* historyList;
    QLabel* historyImageView;
    
    // Current state
    cv::Mat currentImage;
    QString currentImagePath;
    
    // Helper methods
    void setupUI();
    void connectSignals();
    void updateBranchList();
    void updateHistoryList();
    void displayImage(const cv::Mat& image);
    
    QLayout* leftLayout() const;
    QLayout* centerLayout() const;
    QLayout* rightLayout() const;
    
private slots:
    void onOpenRepository();
    void onInitRepository();
    void onAddButtonClicked();
    void onCommitButtonClicked();
    void onDiffButtonClicked();
    void onVersionSelected(int index);
    
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    bool openRepository(const QString& path);
};