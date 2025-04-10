#include "MainWindow.h"
#include "TerminalWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QToolBar>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QLabel>
#include <QScrollArea>
#include <QFileInfo>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
    connectSignals();
    
    // Set window properties
    setWindowTitle("Versionary");
    setMinimumSize(1000, 700);
    
    // Initialize with current directory
    QString currentDir = QDir::currentPath();
    openRepository(currentDir);
}

MainWindow::~MainWindow() {
    // Clean up resources
}

void MainWindow::setupUI() {
    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Create main splitter
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // Create left panel (repository browser)
    QWidget* leftPanel = new QWidget(mainSplitter);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    QLabel* branchLabel = new QLabel("Branches:", leftPanel);
    branchList = new QListWidget(leftPanel);
    
    QLabel* stagingLabel = new QLabel("Staging:", leftPanel);
    stagingList = new QListWidget(leftPanel);
    
    addButton = new QPushButton("Add Image", leftPanel);
    QPushButton* commitButton = new QPushButton("Commit", leftPanel);
    
    leftLayout->addWidget(branchLabel);
    leftLayout->addWidget(branchList);
    leftLayout->addWidget(stagingLabel);
    leftLayout->addWidget(stagingList);
    leftLayout->addWidget(addButton);
    leftLayout->addWidget(commitButton);
    leftPanel->setLayout(leftLayout);
    
    // Create center panel (image viewer)
    QWidget* centerPanel = new QWidget(mainSplitter);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPanel);
    
    QScrollArea* scrollArea = new QScrollArea(centerPanel);
    scrollArea->setWidgetResizable(true);
    
    imageView = new QLabel(scrollArea);
    imageView->setAlignment(Qt::AlignCenter);
    imageView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(imageView);
    
    QPushButton* diffButton = new QPushButton("Show Diff", centerPanel);
    
    centerLayout->addWidget(scrollArea);
    centerLayout->addWidget(diffButton);
    centerPanel->setLayout(centerLayout);
    
    // Create right panel (history)
    QWidget* rightPanel = new QWidget(mainSplitter);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    QLabel* historyLabel = new QLabel("Version History:", rightPanel);
    historyList = new QListWidget(rightPanel);
    
    QScrollArea* historyImageScrollArea = new QScrollArea(rightPanel);
    historyImageScrollArea->setWidgetResizable(true);
    
    historyImageView = new QLabel(historyImageScrollArea);
    historyImageView->setAlignment(Qt::AlignCenter);
    historyImageScrollArea->setWidget(historyImageView);
    
    rightLayout->addWidget(historyLabel);
    rightLayout->addWidget(historyList);
    rightLayout->addWidget(historyImageScrollArea);
    rightPanel->setLayout(rightLayout);
    
    // Add panels to main splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(centerPanel);
    mainSplitter->addWidget(rightPanel);
    
    // Set initial sizes
    mainSplitter->setSizes(QList<int>() << 200 << 500 << 300);
    
    // Create terminal widget
    TerminalWidget* terminal = new TerminalWidget(&cli, centralWidget);
    
    // Add widgets to main layout
    mainLayout->addWidget(mainSplitter);
    mainLayout->addWidget(terminal);
    
    // Set central widget
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    
    // Create menu bar
    QMenuBar* menuBar = new QMenuBar(this);
    
    QMenu* fileMenu = new QMenu("File", menuBar);
    QAction* openAction = new QAction("Open Repository", fileMenu);
    QAction* initAction = new QAction("Initialize Repository", fileMenu);
    QAction* exitAction = new QAction("Exit", fileMenu);
    
    fileMenu->addAction(openAction);
    fileMenu->addAction(initAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    menuBar->addMenu(fileMenu);
    setMenuBar(menuBar);
    
    // Create toolbar
    QToolBar* toolbar = new QToolBar(this);
    toolbar->addAction(openAction);
    toolbar->addAction(initAction);
    addToolBar(toolbar);
    
    // Create status bar
    statusBar()->showMessage("Ready");
}

void MainWindow::connectSignals() {
    // Connect menu actions
    connect(menuBar()->actions().at(0)->menu()->actions().at(0), &QAction::triggered, 
            this, &MainWindow::onOpenRepository);
    connect(menuBar()->actions().at(0)->menu()->actions().at(1), &QAction::triggered, 
            this, &MainWindow::onInitRepository);
    connect(menuBar()->actions().at(0)->menu()->actions().at(3), &QAction::triggered, 
            this, &QApplication::quit);
    
    // Connect buttons
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    connect(static_cast<QPushButton*>(leftLayout()->itemAt(5)->widget()), &QPushButton::clicked, 
            this, &MainWindow::onCommitButtonClicked);
    connect(static_cast<QPushButton*>(centerLayout()->itemAt(1)->widget()), &QPushButton::clicked, 
            this, &MainWindow::onDiffButtonClicked);
    
    // Connect list widgets
    connect(historyList, &QListWidget::currentRowChanged, this, &MainWindow::onVersionSelected);
}

QLayout* MainWindow::leftLayout() const {
    return static_cast<QWidget*>(mainSplitter->widget(0))->layout();
}

QLayout* MainWindow::centerLayout() const {
    return static_cast<QWidget*>(mainSplitter->widget(1))->layout();
}

QLayout* MainWindow::rightLayout() const {
    return static_cast<QWidget*>(mainSplitter->widget(2))->layout();
}

bool MainWindow::openRepository(const QString& path) {
    cli.setCurrentDirectory(path.toStdString());
    
    if (!versionManager.isRepository()) {
        // Not a repository, but don't show error
        statusBar()->showMessage("Not a Versionary repository");
        return false;
    }
    
    // Load repository
    versionManager.loadAllVersions();
    
    // Update UI
    updateBranchList();
    updateHistoryList();
    
    statusBar()->showMessage("Repository opened: " + path);
    return true;
}

void MainWindow::updateBranchList() {
    branchList->clear();
    branchList->addItem(QString::fromStdString(versionManager.getCurrentBranch()));
    branchList->setCurrentRow(0);
}

void MainWindow::updateHistoryList() {
    historyList->clear();
    
    std::vector<Version> history = versionManager.getHistory();
    
    for (const auto& version : history) {
        QString displayText = QString::fromStdString(version.id.substr(0, 8) + " - " + 
                                                    version.timestamp + " - " + 
                                                    version.message);
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, QString::fromStdString(version.id));
        historyList->addItem(item);
    }
    
    if (historyList->count() > 0) {
        historyList->setCurrentRow(0);
    }
}

void MainWindow::displayImage(const cv::Mat& image) {
    if (image.empty()) {
        imageView->clear();
        return;
    }
    
    // Convert OpenCV Mat to QImage
    cv::Mat rgbImage;
    if (image.channels() == 1) {
        cv::cvtColor(image, rgbImage, cv::COLOR_GRAY2RGB);
    } else {
        cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB);
    }
    
    QImage qImage(rgbImage.data, rgbImage.cols, rgbImage.rows, 
                 rgbImage.step, QImage::Format_RGB888);
    
    // Display image
    imageView->setPixmap(QPixmap::fromImage(qImage));
}

void MainWindow::onOpenRepository() {
    QString dir = QFileDialog::getExistingDirectory(this, "Open Repository",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   QFileDialog::ShowDirsOnly);
    
    if (!dir.isEmpty()) {
        openRepository(dir);
    }
}

void MainWindow::onInitRepository() {
    QString dir = QFileDialog::getExistingDirectory(this, "Initialize Repository",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   QFileDialog::ShowDirsOnly);
    
    if (!dir.isEmpty()) {
        cli.setCurrentDirectory(dir.toStdString());
        
        if (versionManager.initRepository(dir.toStdString())) {
            QMessageBox::information(this, "Repository Initialized", 
                                    "Initialized empty Versionary repository in " + dir);
            openRepository(dir);
        } else {
            QMessageBox::critical(this, "Error", "Failed to initialize repository");
        }
    }
}

void MainWindow::onAddButtonClicked() {
    QString imagePath = QFileDialog::getOpenFileName(this, "Add Image",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    "Images (*.png *.jpg *.jpeg *.bmp)");
    
    if (!imagePath.isEmpty()) {
        cv::Mat image = cv::imread(imagePath.toStdString());
        
        if (image.empty()) {
            QMessageBox::critical(this, "Error", "Could not read image " + imagePath);
            return;
        }
        
        if (versionManager.addImage(image)) {
            // Save as staged_image.png
            std::string stagedPath = cli.getCurrentDirectory() + "/staged_image.png";
            cv::imwrite(stagedPath, image);
            
            // Update UI
            stagingList->clear();
            stagingList->addItem(QFileInfo(imagePath).fileName());
            
            // Display image
            currentImage = image;
            currentImagePath = imagePath;
            displayImage(currentImage);
            
            statusBar()->showMessage("Added image: " + imagePath);
        } else {
            QMessageBox::critical(this, "Error", "Failed to add image");
        }
    }
}

void MainWindow::onCommitButtonClicked() {
    if (stagingList->count() == 0) {
        QMessageBox::warning(this, "Warning", "No changes to commit");
        return;
    }
    
    bool ok;
    QString message = QInputDialog::getText(this, "Commit",
                                          "Commit message:", QLineEdit::Normal,
                                          "", &ok);
    
    if (ok && !message.isEmpty()) {
        std::string stagedPath = cli.getCurrentDirectory() + "/staged_image.png";
        cv::Mat stagedImage = cv::imread(stagedPath);
        
        if (stagedImage.empty()) {
            QMessageBox::critical(this, "Error", "No image staged for commit");
            return;
        }
        
        std::string versionId = versionManager.commit(message.toStdString(), stagedImage);
        
        if (!versionId.empty()) {
            QMessageBox::information(this, "Commit Successful", 
                                    "Created version " + QString::fromStdString(versionId));
            
            // Clear staging
            stagingList->clear();
            
            // Remove staged image
            QFile::remove(QString::fromStdString(stagedPath));
            
            // Update history
            updateHistoryList();
            
            statusBar()->showMessage("Committed: " + message);
        } else {
            QMessageBox::critical(this, "Error", "Failed to create version");
        }
    }
}

void MainWindow::onDiffButtonClicked() {
    if (historyList->currentRow() < 0) {
        QMessageBox::warning(this, "Warning", "No version selected for diff");
        return;
    }
    
    QString versionId = historyList->currentItem()->data(Qt::UserRole).toString();
    
    // Check if we have a staged image
    std::string stagedPath = cli.getCurrentDirectory() + "/staged_image.png";
    cv::Mat stagedImage = cv::imread(stagedPath);
    
    cv::Mat diffImage;
    
    if (!stagedImage.empty()) {
        // Diff between staged and selected version
        diffImage = versionManager.getDiff(versionId.toStdString(), versionManager.getHeadVersionId());
    } else if (!currentImage.empty()) {
        // Diff between current and selected version
        diffImage = versionManager.getDiff(versionId.toStdString(), versionManager.getHeadVersionId());
    } else {
        // Diff between selected version and HEAD
        diffImage = versionManager.getDiff(versionId.toStdString(), versionManager.getHeadVersionId());
    }
    
    if (diffImage.empty()) {
        QMessageBox::critical(this, "Error", "Failed to generate diff");
        return;
    }
    
    // Display diff image
    displayImage(diffImage);
    statusBar()->showMessage("Showing diff for version " + versionId);
}

void MainWindow::onVersionSelected(int index) {
    if (index < 0) {
        return;
    }
    
    QString versionId = historyList->item(index)->data(Qt::UserRole).toString();
    
    // Load version image
    cv::Mat versionImage = versionManager.loadImage(versionId.toStdString());
    
    if (!versionImage.empty()) {
        // Convert OpenCV Mat to QImage for history view
        cv::Mat rgbImage;
        if (versionImage.channels() == 1) {
            cv::cvtColor(versionImage, rgbImage, cv::COLOR_GRAY2RGB);
        } else {
            cv::cvtColor(versionImage, rgbImage, cv::COLOR_BGR2RGB);
        }
        
        QImage qImage(rgbImage.data, rgbImage.cols, rgbImage.rows, 
                     rgbImage.step, QImage::Format_RGB888);
        
        // Display image in history view
        historyImageView->setPixmap(QPixmap::fromImage(qImage));
        
        // Also show in main view if no current image
        if (currentImage.empty()) {
            displayImage(versionImage);
        }
        
        statusBar()->showMessage("Selected version: " + versionId);
    }
}

std::string VersionaryCLI::getCurrentDirectory() const {
    return currentDir;
}