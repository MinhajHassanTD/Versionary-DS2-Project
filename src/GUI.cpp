#include "GUI.h"
#include <QDateTime>
#include <QStandardPaths>

GUI::GUI(VersionControl& versionControl, QWidget* parent)
    : QMainWindow(parent), versionControl_(versionControl) {

    setWindowTitle("Versionary - Image Version Control");
    resize(1200, 800);

    createUI();
    createMenuBar();
    createToolBar();

    updateVersionList();

    // Set current version
    VersionInfo currentVersion = versionControl_.getCurrentVersion();
    if (!currentVersion.id.empty()) {
        currentVersionId_ = currentVersion.id;
        statusBar_->showMessage("Current version: " + QString::fromStdString(currentVersionId_));

        // Display current version image
        cv::Mat image = versionControl_.getVersionImage(currentVersionId_);
        if (!image.empty()) {
            displayImage(image, imageLabel_);
        }
    }
}

GUI::~GUI() {
}

void GUI::openRepository() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Repository",
                                                      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                      QFileDialog::ShowDirsOnly);

    if (dirPath.isEmpty()) {
        return;
    }

    // Create new version control with selected directory
    versionControl_ = VersionControl(dirPath.toStdString());

    // Update UI
    updateVersionList();

    // Set current version
    VersionInfo currentVersion = versionControl_.getCurrentVersion();
    if (!currentVersion.id.empty()) {
        currentVersionId_ = currentVersion.id;
        statusBar_->showMessage("Current version: " + QString::fromStdString(currentVersionId_));

        // Display current version image
        cv::Mat image = versionControl_.getVersionImage(currentVersionId_);
        if (!image.empty()) {
            displayImage(image, imageLabel_);
        }
    }

    showSuccess("Repository opened: " + dirPath);
}

void GUI::createRepository() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Create Repository",
                                                      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                      QFileDialog::ShowDirsOnly);

    if (dirPath.isEmpty()) {
        return;
    }

    // Create new version control with selected directory
    versionControl_ = VersionControl(dirPath.toStdString());

    // Initialize repository
    if (versionControl_.initRepository()) {
        showSuccess("Repository created: " + dirPath);
    } else {
        showError("Failed to create repository: " + dirPath);
    }

    // Update UI
    updateVersionList();
}

void GUI::addImage() {
    QString filePath = QFileDialog::getOpenFileName(this, "Add Image",
                                                  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                  "Images (*.png *.jpg *.jpeg *.bmp)");

    if (filePath.isEmpty()) {
        return;
    }

    if (versionControl_.addImage(filePath.toStdString())) {
        showSuccess("Image added to staging area: " + filePath);

        // Load and display the image
        cv::Mat image = cv::imread(filePath.toStdString());
        if (!image.empty()) {
            displayImage(image, imageLabel_);
        }
    } else {
        showError("Failed to add image: " + filePath);
    }
}

void GUI::commitImage() {
    QString message = commitMessageEdit_->text();

    if (message.isEmpty()) {
        showError("Please enter a commit message.");
        return;
    }

    // Get encryption and signing options
    bool encrypt = encryptCheckBox_->isChecked();
    bool sign = signCheckBox_->isChecked();

    std::string versionId = versionControl_.commitImage(message.toStdString(), "", encrypt, sign);

    if (!versionId.empty()) {
        QString successMessage = "Committed version: " + QString::fromStdString(versionId);

        if (encrypt) {
            successMessage += " (encrypted)";
        }

        if (sign) {
            successMessage += " (signed)";
        }

        showSuccess(successMessage);

        // Update UI
        updateVersionList();

        // Clear commit message
        commitMessageEdit_->clear();

        // Update current version
        currentVersionId_ = versionId;
        statusBar_->showMessage("Current version: " + QString::fromStdString(currentVersionId_));
    } else {
        showError("Failed to commit image.");
    }
}

void GUI::versionSelected(QListWidgetItem* item) {
    if (!item) {
        return;
    }

    // Extract version ID from item text
    QString text = item->text();
    int idEnd = text.indexOf(" - ");

    if (idEnd == -1) {
        return;
    }

    QString versionId = text.left(idEnd);
    selectedVersionId_ = versionId.toStdString();

    // Display selected version image
    cv::Mat image = versionControl_.getVersionImage(selectedVersionId_);
    if (!image.empty()) {
        displayImage(image, imageLabel_);
    }

    // Update compare version combo box
    compareVersionCombo_->clear();

    std::vector<VersionInfo> versions = versionControl_.getAllVersions();
    for (const auto& version : versions) {
        if (version.id != selectedVersionId_) {
            QString itemText = QString::fromStdString(version.id) + " - " +
                             QString::fromStdString(version.timestamp) + " - " +
                             QString::fromStdString(version.message);
            compareVersionCombo_->addItem(itemText);
        }
    }

    // Enable buttons
    compareButton_->setEnabled(true);
    rollbackButton_->setEnabled(true);
    visualizeButton_->setEnabled(true);
}

void GUI::compareVersions() {
    if (selectedVersionId_.empty() || compareVersionCombo_->currentIndex() == -1) {
        showError("Please select two versions to compare.");
        return;
    }

    // Extract version ID from combo box
    QString text = compareVersionCombo_->currentText();
    int idEnd = text.indexOf(" - ");

    if (idEnd == -1) {
        return;
    }

    QString versionId = text.left(idEnd);
    std::string compareVersionId = versionId.toStdString();

    // Compare versions
    cv::Mat diffImage = versionControl_.compareVersions(selectedVersionId_, compareVersionId);

    if (!diffImage.empty()) {
        displayImage(diffImage, diffImageLabel_);
    } else {
        showError("Failed to compare versions.");
    }
}

void GUI::rollbackToVersion() {
    if (selectedVersionId_.empty()) {
        showError("Please select a version to roll back to.");
        return;
    }

    // Confirm rollback
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Rollback",
                                                           "Are you sure you want to roll back to version " +
                                                           QString::fromStdString(selectedVersionId_) + "?",
                                                           QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    if (versionControl_.rollbackToVersion(selectedVersionId_)) {
        showSuccess("Rolled back to version: " + QString::fromStdString(selectedVersionId_));

        // Update current version
        currentVersionId_ = selectedVersionId_;
        statusBar_->showMessage("Current version: " + QString::fromStdString(currentVersionId_));
    } else {
        showError("Failed to roll back to version: " + QString::fromStdString(selectedVersionId_));
    }
}

void GUI::visualizeQuadtree() {
    if (selectedVersionId_.empty()) {
        showError("Please select a version to visualize.");
        return;
    }

    // Get image for selected version
    cv::Mat image = versionControl_.getVersionImage(selectedVersionId_);

    if (image.empty()) {
        showError("Failed to load image for version: " + QString::fromStdString(selectedVersionId_));
        return;
    }

    // Create image processor and visualize Quadtree
    ImageProcessor processor;
    processor.loadImage(versionControl_.getVersion(selectedVersionId_).imagePath);
    cv::Mat visualized = processor.visualizeQuadtree();

    if (!visualized.empty()) {
        displayImage(visualized, diffImageLabel_);
    } else {
        showError("Failed to visualize Quadtree.");
    }
}

void GUI::about() {
    QMessageBox::about(this, "About Versionary",
                     "Versionary: An Image-Based Version Control System\n\n"
                     "Developed by:\n"
                     "- Azfar Ali (aa08861)\n"
                     "- Minhaj ul Hassan (mu08984)\n"
                     "- Muhammad Qasim Khan (mk05539)\n\n"
                     "Instructor: Dr. Usman Arif\n\n"
                     "Features:\n"
                     "- Image version tracking with Merkle Trees\n"
                     "- Quadtree-based image comparison\n"
                     "- Branching and merging\n"
                     "- Encryption and digital signatures\n\n"
                     "© 2025 - Data Structures 2 Project");
}

void GUI::createUI() {
    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create main layout
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // Create left panel for version list
    QGroupBox* versionGroupBox = new QGroupBox("Versions", centralWidget);
    QVBoxLayout* versionLayout = new QVBoxLayout(versionGroupBox);

    versionList_ = new QListWidget(versionGroupBox);
    versionLayout->addWidget(versionList_);

    // Create commit controls
    QGroupBox* commitGroupBox = new QGroupBox("Commit", centralWidget);
    QVBoxLayout* commitLayout = new QVBoxLayout(commitGroupBox);

    commitMessageEdit_ = new QLineEdit(commitGroupBox);
    commitMessageEdit_->setPlaceholderText("Enter commit message");
    commitLayout->addWidget(commitMessageEdit_);

    // Add security options
    QHBoxLayout* securityLayout = new QHBoxLayout();

    encryptCheckBox_ = new QCheckBox("Encrypt", commitGroupBox);
    signCheckBox_ = new QCheckBox("Sign", commitGroupBox);

    securityLayout->addWidget(encryptCheckBox_);
    securityLayout->addWidget(signCheckBox_);
    securityLayout->addStretch();

    commitLayout->addLayout(securityLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    addButton_ = new QPushButton("Add Image", commitGroupBox);
    commitButton_ = new QPushButton("Commit", commitGroupBox);

    buttonLayout->addWidget(addButton_);
    buttonLayout->addWidget(commitButton_);

    commitLayout->addLayout(buttonLayout);

    // Create compare controls
    QGroupBox* compareGroupBox = new QGroupBox("Compare", centralWidget);
    QVBoxLayout* compareLayout = new QVBoxLayout(compareGroupBox);

    compareVersionCombo_ = new QComboBox(compareGroupBox);
    compareLayout->addWidget(compareVersionCombo_);

    QHBoxLayout* compareButtonLayout = new QHBoxLayout();

    compareButton_ = new QPushButton("Compare", compareGroupBox);
    rollbackButton_ = new QPushButton("Rollback", compareGroupBox);
    visualizeButton_ = new QPushButton("Visualize Quadtree", compareGroupBox);

    compareButtonLayout->addWidget(compareButton_);
    compareButtonLayout->addWidget(rollbackButton_);
    compareButtonLayout->addWidget(visualizeButton_);

    compareLayout->addLayout(compareButtonLayout);

    // Disable buttons initially
    compareButton_->setEnabled(false);
    rollbackButton_->setEnabled(false);
    visualizeButton_->setEnabled(false);

    // Create left panel layout
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(versionGroupBox);
    leftLayout->addWidget(commitGroupBox);
    leftLayout->addWidget(compareGroupBox);

    // Create right panel for image display
    QSplitter* imageSplitter = new QSplitter(Qt::Vertical, centralWidget);

    // Create image display
    QGroupBox* imageGroupBox = new QGroupBox("Image", centralWidget);
    QVBoxLayout* imageLayout = new QVBoxLayout(imageGroupBox);

    imageScrollArea_ = new QScrollArea(imageGroupBox);
    imageScrollArea_->setWidgetResizable(true);
    imageScrollArea_->setMinimumHeight(300);

    imageLabel_ = new QLabel(imageScrollArea_);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    imageScrollArea_->setWidget(imageLabel_);
    imageLayout->addWidget(imageScrollArea_);

    // Create diff image display
    QGroupBox* diffImageGroupBox = new QGroupBox("Difference / Visualization", centralWidget);
    QVBoxLayout* diffImageLayout = new QVBoxLayout(diffImageGroupBox);

    diffImageScrollArea_ = new QScrollArea(diffImageGroupBox);
    diffImageScrollArea_->setWidgetResizable(true);
    diffImageScrollArea_->setMinimumHeight(300);

    diffImageLabel_ = new QLabel(diffImageScrollArea_);
    diffImageLabel_->setAlignment(Qt::AlignCenter);
    diffImageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    diffImageScrollArea_->setWidget(diffImageLabel_);
    diffImageLayout->addWidget(diffImageScrollArea_);

    imageSplitter->addWidget(imageGroupBox);
    imageSplitter->addWidget(diffImageGroupBox);

    // Add panels to main layout
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addWidget(imageSplitter, 2);

    // Create status bar
    statusBar_ = new QStatusBar(this);
    setStatusBar(statusBar_);

    // Connect signals and slots
    connect(versionList_, &QListWidget::itemClicked, this, &GUI::versionSelected);
    connect(addButton_, &QPushButton::clicked, this, &GUI::addImage);
    connect(commitButton_, &QPushButton::clicked, this, &GUI::commitImage);
    connect(compareButton_, &QPushButton::clicked, this, &GUI::compareVersions);
    connect(rollbackButton_, &QPushButton::clicked, this, &GUI::rollbackToVersion);
    connect(visualizeButton_, &QPushButton::clicked, this, &GUI::visualizeQuadtree);
}

void GUI::createMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // File menu
    QMenu* fileMenu = menuBar->addMenu("File");

    QAction* openAction = fileMenu->addAction("Open Repository");
    connect(openAction, &QAction::triggered, this, &GUI::openRepository);

    QAction* createAction = fileMenu->addAction("Create Repository");
    connect(createAction, &QAction::triggered, this, &GUI::createRepository);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Help menu
    QMenu* helpMenu = menuBar->addMenu("Help");

    QAction* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &GUI::about);
}

void GUI::createToolBar() {
    QToolBar* toolBar = new QToolBar(this);
    addToolBar(toolBar);

    QAction* openAction = toolBar->addAction("Open Repository");
    connect(openAction, &QAction::triggered, this, &GUI::openRepository);

    QAction* createAction = toolBar->addAction("Create Repository");
    connect(createAction, &QAction::triggered, this, &GUI::createRepository);

    toolBar->addSeparator();

    QAction* addAction = toolBar->addAction("Add Image");
    connect(addAction, &QAction::triggered, this, &GUI::addImage);

    QAction* commitAction = toolBar->addAction("Commit");
    connect(commitAction, &QAction::triggered, this, &GUI::commitImage);
}

void GUI::updateVersionList() {
    versionList_->clear();

    std::vector<VersionInfo> versions = versionControl_.getAllVersions();

    for (const auto& version : versions) {
        QString itemText = QString::fromStdString(version.id) + " - " +
                         QString::fromStdString(version.timestamp) + " - " +
                         QString::fromStdString(version.message);

        QListWidgetItem* item = new QListWidgetItem(itemText, versionList_);

        // Highlight current version
        if (version.id == currentVersionId_) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setBackground(QColor(200, 230, 255));
        }
    }
}

void GUI::displayImage(const cv::Mat& image, QLabel* label) {
    if (image.empty()) {
        return;
    }

    QImage qImage = matToQImage(image);
    QPixmap pixmap = QPixmap::fromImage(qImage);

    label->setPixmap(pixmap);
    label->setMinimumSize(pixmap.size());
}

QImage GUI::matToQImage(const cv::Mat& image) {
    cv::Mat rgb;

    if (image.channels() == 1) {
        cv::cvtColor(image, rgb, cv::COLOR_GRAY2RGB);
    } else if (image.channels() == 3) {
        cv::cvtColor(image, rgb, cv::COLOR_BGR2RGB);
    } else if (image.channels() == 4) {
        cv::cvtColor(image, rgb, cv::COLOR_BGRA2RGB);
    } else {
        return QImage();
    }

    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
}

void GUI::showError(const QString& message) {
    QMessageBox::critical(this, "Error", message);
    statusBar_->showMessage("Error: " + message, 5000);
}

void GUI::showSuccess(const QString& message) {
    statusBar_->showMessage(message, 5000);
}
