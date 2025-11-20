#include "MainWindow.h"
#include "ImageCanvas.h"

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFrame>
#include <QFileInfo>
#include <QSizePolicy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_pages(new QStackedWidget(this))
    , m_homePage(nullptr)
    , m_uploadButton(nullptr)
    , m_infoLabel(nullptr)
    , m_previewPage(nullptr)
    , m_originalImageLabel(nullptr)
    , m_blurredImageCanvas(nullptr)
    , m_detectButton(nullptr)
    , m_manualEditButton(nullptr)
    , m_exportButton(nullptr)
    , m_undoButton(nullptr)
    , m_redoButton(nullptr)
    , m_blurSlider(nullptr)
    , m_manualEditEnabled(false)
{
    // Create both pages
    createHomePage();
    createPreviewPage();

    // Start on the home page
    m_pages->setCurrentWidget(m_homePage);
    setCentralWidget(m_pages);

    setWindowTitle("CleanShare");
    resize(1200, 800);

    // Terms of Service dialog on startup
    QString tosText =
        "CleanShare is a local tool that helps you blur sensitive information "
        "in images before sharing.\n\n"
        "Please confirm that you understand that:\n"
        "  • You are responsible for the images you load and share.\n"
        "  • This tool does not upload your images to any server.\n"
        "  • Use this tool responsibly; it can blur sensitive details, but it has practical limits.\n\n"
        "Do you accept these terms?";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Terms of Service",
        tosText,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (reply != QMessageBox::Yes) {
        QTimer::singleShot(0, this, &QWidget::close);
    }
}

// ---------------- Home page ----------------

void MainWindow::createHomePage()
{
    m_homePage = new QWidget(this);
    m_uploadButton = new QPushButton("Upload image", this);
    m_infoLabel = new QLabel("No file selected", this);

    QVBoxLayout *homeLayout = new QVBoxLayout(m_homePage);

    homeLayout->addStretch();

    m_uploadButton->setMinimumHeight(40);
    homeLayout->addWidget(m_uploadButton, 0, Qt::AlignHCenter);

    m_infoLabel->setAlignment(Qt::AlignCenter);
    homeLayout->addWidget(m_infoLabel, 0, Qt::AlignHCenter);

    homeLayout->addStretch();

    m_homePage->setLayout(homeLayout);

    m_pages->addWidget(m_homePage);

    connect(m_uploadButton, &QPushButton::clicked,
            this, &MainWindow::onUploadClicked);
}

void MainWindow::onUploadClicked()
{
    QString filter = "Images (*.png *.jpg *.jpeg *.bmp)";
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select an image",
        QString(),
        filter
        );

    if (filePath.isEmpty()) {
        m_infoLabel->setText("No file selected");
        return;
    }

    // Ask the controller to load the image
    if (!m_session.loadImage(filePath)) {
        QMessageBox::warning(
            this,
            "Error",
            "Failed to load image:\n" + filePath
            );
        return;
    }

    // Update label on home page
    m_infoLabel->setText("Selected file:\n" + filePath);

    m_currentImagePath = filePath;

    // Show the controller's pixmaps in both panels
    showImageInPanels();

    // Switch to preview page
    m_pages->setCurrentWidget(m_previewPage);
}

// ---------------- Preview page ----------------

void MainWindow::createPreviewPage()
{
    m_previewPage = new QWidget(this);

    // --- Top toolbar area (buttons + slider) ---
    m_detectButton      = new QPushButton("Detect & Blur", this);
    m_manualEditButton  = new QPushButton("Manual Edit: Off", this);
    m_exportButton      = new QPushButton("Export", this);
    m_undoButton        = new QPushButton("Undo", this);
    m_redoButton        = new QPushButton("Redo", this);
    m_blurSlider        = new QSlider(Qt::Horizontal, this);

    m_blurSlider->setRange(0, 100);
    m_blurSlider->setValue(50);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(m_detectButton);
    toolbarLayout->addWidget(m_manualEditButton);
    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(m_undoButton);
    toolbarLayout->addWidget(m_redoButton);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(new QLabel("Blur strength:", this));
    toolbarLayout->addWidget(m_blurSlider);
    toolbarLayout->addStretch();

    // --- Middle image area: Original | Blurred ---
    m_originalImageLabel = new QLabel("Original image will appear here.", this);
    m_originalImageLabel->setAlignment(Qt::AlignCenter);
    m_originalImageLabel->setFrameStyle(QFrame::Box | QFrame::Plain);

    m_blurredImageCanvas = new ImageCanvas(this);

    // Give both sides the same behavior and minimum sizes
    m_originalImageLabel->setMinimumSize(400, 400);
    m_blurredImageCanvas->setMinimumSize(400, 400);

    m_originalImageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_blurredImageCanvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Single images layout (this is where you had the bug before)
    QHBoxLayout *imagesLayout = new QHBoxLayout();
    imagesLayout->addWidget(m_originalImageLabel, 1);      // left stretch = 1
    imagesLayout->addWidget(m_blurredImageCanvas, 1);      // right stretch = 1

    // --- Put it all together ---
    QVBoxLayout *previewLayout = new QVBoxLayout(m_previewPage);
    previewLayout->addLayout(toolbarLayout);
    previewLayout->addSpacing(10);
    previewLayout->addLayout(imagesLayout);
    // optional: give image area more vertical stretch
    // previewLayout->setStretch(0, 0);
    // previewLayout->setStretch(2, 1);

    m_previewPage->setLayout(previewLayout);

    // Connect preview controls
    connect(m_detectButton, &QPushButton::clicked,
            this, &MainWindow::onDetectClicked);

    connect(m_blurSlider, &QSlider::valueChanged,
            this, &MainWindow::onBlurSliderChanged);

    connect(m_exportButton, &QPushButton::clicked,
            this, &MainWindow::onExportClicked);

    connect(m_manualEditButton, &QPushButton::clicked,
            this, &MainWindow::onManualEditClicked);

    m_pages->addWidget(m_previewPage);
}

void MainWindow::showImageInPanels()
{
    if (!m_session.hasImage()) {
        return;
    }

    m_originalPixmap = m_session.originalPixmap();
    m_blurredPixmap  = m_session.blurredPixmap();

    updatePreviewLabels();
}

void MainWindow::updatePreviewLabels()
{
    if (m_originalPixmap.isNull())
        return;

    // Scale to fit the left label
    QPixmap origScaled = m_originalPixmap.scaled(
        m_originalImageLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    m_originalImageLabel->setPixmap(origScaled);
    m_originalImageLabel->setText(QString());

    // Right side: pass the blurred pixmap to the canvas
    m_blurredImageCanvas->setImage(m_blurredPixmap);
}

// ---------------- Blur logic (dummy) ----------------

void MainWindow::applyFakeBlur(int strength)
{
    if (!m_session.hasImage())
        return;

    m_session.applyFakeBlur(strength);

    // Refresh local pixmaps from session
    m_originalPixmap = m_session.originalPixmap();
    m_blurredPixmap  = m_session.blurredPixmap();

    updatePreviewLabels();
}

void MainWindow::onDetectClicked()
{
    if (!m_session.hasImage()) {
        QMessageBox::information(
            this,
            "No image",
            "Please upload an image first."
            );
        return;
    }

    int strength = m_blurSlider->value();
    applyFakeBlur(strength);
}

void MainWindow::onBlurSliderChanged(int value)
{
    Q_UNUSED(value);

    if (!m_session.hasImage())
        return;

    // Re-blur from the original each time
    applyFakeBlur(m_blurSlider->value());
}

// ---------------- Export ----------------

void MainWindow::onExportClicked()
{
    if (!m_session.hasImage()) {
        QMessageBox::information(
            this,
            "Nothing to export",
            "There is no blurred image to export yet.\n\n"
            "Try clicking \"Detect & Blur\" first."
            );
        return;
    }

    // Suggest a filename based on the input image
    QString suggestedName = "cleanshare_output.png";
    if (!m_currentImagePath.isEmpty()) {
        QFileInfo info(m_currentImagePath);
        suggestedName = info.completeBaseName() + "_cleaned.png";
    }

    QString filter = "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg)";
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Export blurred image",
        suggestedName,
        filter
        );

    if (savePath.isEmpty()) {
        return;
    }

    if (!m_session.blurredPixmap().save(savePath)) {
        QMessageBox::warning(
            this,
            "Export failed",
            "Could not save image to:\n" + savePath
            );
        return;
    }

    QMessageBox::information(
        this,
        "Export successful",
        "Blurred image exported to:\n" + savePath
        );
}

// ---------------- Manual edit toggle ----------------

void MainWindow::onManualEditClicked()
{
    m_manualEditEnabled = !m_manualEditEnabled;
    m_blurredImageCanvas->setEditingEnabled(m_manualEditEnabled);

    m_manualEditButton->setText(
        m_manualEditEnabled ? "Manual Edit: On" : "Manual Edit: Off"
        );
}
