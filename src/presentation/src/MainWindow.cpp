#include "MainWindow.h"
#include "ImageCanvas.h"

#include <QStackedWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QFrame>
#include <QFileInfo>
#include <QSizePolicy>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QKeyEvent>

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
    , m_blurDebounceTimer(nullptr)
    , m_pendingBlurValue(50)
    , m_lastSelectionWasAddMode(true)
    , m_blurValueLabel(nullptr)
    , m_blurWatcher(nullptr)
    , m_lastBlurJobStrength(-1)
    , m_manualEditEnabled(false)
{
    setAcceptDrops(true);
    
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

    setStyleSheet(
    "QWidget#DropArea {"
    "    border: 2px dashed #999999;"
    "    border-radius: 12px;"
    "    background-color: rgba(0, 0, 0, 10);"
    "}"
    "QWidget#DropArea:hover {"
    "    border-color: #0078d4;"
    "    background-color: rgba(0, 120, 212, 15);"
    "}"
);

}

// ---------------- Home page ----------------

void MainWindow::createHomePage()
{
    m_homePage = new QWidget(this);

    // Main layout for the whole page
    auto *homeLayout = new QVBoxLayout(m_homePage);
    homeLayout->setContentsMargins(40, 40, 40, 40);
    homeLayout->setSpacing(20);
    homeLayout->setAlignment(Qt::AlignCenter);

    // Drop area widget with dashed border
    QWidget *dropArea = new QWidget(m_homePage);
    dropArea->setObjectName("DropArea");       // used in stylesheet
    dropArea->setMinimumSize(400, 250);

    auto *dropLayout = new QVBoxLayout(dropArea);
    dropLayout->setContentsMargins(30, 30, 30, 30);
    dropLayout->setSpacing(10);
    dropLayout->setAlignment(Qt::AlignCenter);

    // Text inside the drop area
    QLabel *titleLabel = new QLabel("Drag and drop an image here", dropArea);
    titleLabel->setAlignment(Qt::AlignCenter);

    m_infoLabel = new QLabel("or click the button below to choose a file", dropArea);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setWordWrap(true);

    dropLayout->addWidget(titleLabel);
    dropLayout->addWidget(m_infoLabel);

    // Upload button below the drop area
    m_uploadButton = new QPushButton("Upload image", m_homePage);
    m_uploadButton->setMinimumHeight(40);

    // Add widgets to the page layout
    homeLayout->addWidget(dropArea, 0, Qt::AlignCenter);
    homeLayout->addWidget(m_uploadButton, 0, Qt::AlignHCenter);

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
    // Clear any previous selection because this is a new image
    if (m_blurredImageCanvas) m_blurredImageCanvas->clearSelection();
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
    m_selectReplaceButton = new QToolButton(this);
    m_selectAddButton     = new QToolButton(this);
    m_selectSubtractButton= new QToolButton(this);
    m_blurSpinBox         = new QSpinBox(this);
    m_blurSlider        = new QSlider(Qt::Horizontal, this);

    m_blurSlider->setRange(0, 100);
    m_blurSlider->setValue(50);
    m_blurSlider->setEnabled(false);  // Disable until first blur

    // Selection mode buttons (Replace / Add / Subtract)
    m_selectReplaceButton->setText("Replace");
    m_selectReplaceButton->setCheckable(true);
    m_selectAddButton->setText("Add");
    m_selectAddButton->setCheckable(true);
    m_selectSubtractButton->setText("Subtract");
    m_selectSubtractButton->setCheckable(true);

    QButtonGroup *selGroup = new QButtonGroup(this);
    selGroup->setExclusive(true);
    selGroup->addButton(m_selectReplaceButton);
    selGroup->addButton(m_selectAddButton);
    selGroup->addButton(m_selectSubtractButton);
    // Default: Replace
    m_selectReplaceButton->setChecked(true);

    // Numeric spinbox for precise blur percent
    m_blurSpinBox->setRange(0, 100);
    m_blurSpinBox->setValue(50);
    m_blurSpinBox->setMinimumWidth(70);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(m_detectButton);
    toolbarLayout->addWidget(m_manualEditButton);
    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(m_undoButton);
    toolbarLayout->addWidget(m_redoButton);
    toolbarLayout->addWidget(m_selectReplaceButton);
    toolbarLayout->addWidget(m_selectAddButton);
    toolbarLayout->addWidget(m_selectSubtractButton);
    toolbarLayout->addSpacing(20);
    toolbarLayout->addWidget(new QLabel("Blur strength:", this));
    toolbarLayout->addWidget(m_blurSlider);
    // numeric label showing percentage
    m_blurValueLabel = new QLabel("50%", this);
    m_blurValueLabel->setMinimumWidth(40);
    toolbarLayout->addWidget(m_blurValueLabel);
    toolbarLayout->addWidget(m_blurSpinBox);
    toolbarLayout->addStretch();

    // Create debounce timer for blur slider
    m_blurDebounceTimer = new QTimer(this);
    m_blurDebounceTimer->setSingleShot(true);

        // Background blur watcher
        m_blurWatcher = new QFutureWatcher<QPixmap>(this);
        connect(m_blurWatcher, &QFutureWatcher<QPixmap>::finished,
            this, &MainWindow::onBackgroundBlurFinished);

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

        connect(m_blurSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &MainWindow::onBlurSpinChanged);

        connect(m_selectReplaceButton, &QToolButton::clicked,
            this, &MainWindow::onSelectReplaceClicked);
        connect(m_selectAddButton, &QToolButton::clicked,
            this, &MainWindow::onSelectAddClicked);
        connect(m_selectSubtractButton, &QToolButton::clicked,
            this, &MainWindow::onSelectSubtractClicked);

    connect(m_blurDebounceTimer, &QTimer::timeout,
            this, &MainWindow::onBlurDebounceTimeout);

    connect(m_blurSlider, &QSlider::sliderReleased,
            this, &MainWindow::onBlurDebounceTimeout);

    connect(m_exportButton, &QPushButton::clicked,
            this, &MainWindow::onExportClicked);

    connect(m_manualEditButton, &QPushButton::clicked,
            this, &MainWindow::onManualEditClicked);

    connect(m_undoButton, &QPushButton::clicked,
        this, &MainWindow::onUndoClicked);

    connect(m_redoButton, &QPushButton::clicked,
        this, &MainWindow::onRedoClicked);

        connect(m_blurredImageCanvas, &ImageCanvas::selectionChanged,
            this, &MainWindow::onSelectionChanged);

        connect(m_blurredImageCanvas, &ImageCanvas::selectionModeChanged,
            this, &MainWindow::onSelectionModeChanged);

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

void MainWindow::onBlurDebounceTimeout()
{
    if (!m_session.hasImage())
        return;

    // Start background job to compute full-size blur so UI stays responsive.
    int strength = m_pendingBlurValue;
    m_lastBlurJobStrength = strength;

    // Record undo state before starting background computation
    m_session.pushState();

    // If the user has an active selection, only blur that selection.
    QImage selMask;
    if (m_blurredImageCanvas) selMask = m_blurredImageCanvas->selectionMask();

    if (!selMask.isNull()) {
        // Selection exists -> run masked blur
        QFuture<QPixmap> fut = QtConcurrent::run([sessionCopy = m_session, strength, selMask]() mutable -> QPixmap {
            sessionCopy.applyFakeBlur(strength, selMask);
            return sessionCopy.blurredPixmap();
        });
        m_blurWatcher->setFuture(fut);
    } else {
        // No selection -> full-image blur
        QFuture<QPixmap> fut = QtConcurrent::run([sessionCopy = m_session, strength]() mutable -> QPixmap {
            sessionCopy.applyFakeBlur(strength);
            return sessionCopy.blurredPixmap();
        });
        m_blurWatcher->setFuture(fut);
    }
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

    const int strength = m_blurSlider->value();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString error;
    bool ok = m_session.autoBlurWithPythonDetections(strength, &error);
    QApplication::restoreOverrideCursor();

    if (!ok) {
        if (error.isEmpty()) {
            error = "Python detector failed. Check that Python, the model .pt file, and the ultralytics package are installed.";
        }
        QMessageBox::warning(
            this,
            "Detection failed",
            error
        );
        return;
    }

    // We just modified the session's blurred image; refresh the preview
    showImageInPanels();

    // Ensure slider is enabled now that some blur exists
    if (!m_blurSlider->isEnabled())
        m_blurSlider->setEnabled(true);
}


void MainWindow::onBackgroundBlurFinished()
{
    if (!m_blurWatcher)
        return;

    QPixmap result = m_blurWatcher->result();

    // Commit computed blur into session state and update UI
    m_session.adoptComputedFullBlur(result, m_lastBlurJobStrength);
    m_originalPixmap = m_session.originalPixmap();
    m_blurredPixmap = m_session.blurredPixmap();
    updatePreviewLabels();

    // Ensure slider is enabled after first blur
    if (!m_blurSlider->isEnabled())
        m_blurSlider->setEnabled(true);
}

void MainWindow::onBlurSliderChanged(int value)
{
    // Update the displayed numeric label immediately
    if (m_blurValueLabel)
        m_blurValueLabel->setText(QString::number(value) + "%");

    // Just record the pending value and restart the debounce timer
    m_pendingBlurValue = value;

    // Restart the timer (100ms debounce delay)
    if (m_blurDebounceTimer) {
        m_blurDebounceTimer->stop();
        m_blurDebounceTimer->start(100);  // 100ms delay
    }

    // Keep spinbox in sync (avoid feedback loop)
    if (m_blurSpinBox && m_blurSpinBox->value() != value) {
        m_blurSpinBox->blockSignals(true);
        m_blurSpinBox->setValue(value);
        m_blurSpinBox->blockSignals(false);
    }
}

void MainWindow::onBlurSpinChanged(int value)
{
    // Mirror into slider (this will trigger onBlurSliderChanged)
    if (m_blurSlider && m_blurSlider->value() != value) {
        m_blurSlider->setValue(value);
    }
}

void MainWindow::onSelectReplaceClicked(bool checked)
{
    if (!m_blurredImageCanvas)
        return;

    if (checked) {
        m_blurredImageCanvas->setReplaceMode(true);
        m_blurredImageCanvas->setAddMode(true);
        m_lastSelectionWasAddMode = true;
    }
}

void MainWindow::onSelectAddClicked(bool checked)
{
    if (!m_blurredImageCanvas)
        return;

    if (checked) {
        m_blurredImageCanvas->setReplaceMode(false);
        m_blurredImageCanvas->setAddMode(true);
        m_lastSelectionWasAddMode = true;
    }
}

void MainWindow::onSelectSubtractClicked(bool checked)
{
    if (!m_blurredImageCanvas)
        return;

    if (checked) {
        m_blurredImageCanvas->setReplaceMode(false);
        m_blurredImageCanvas->setAddMode(false);
        m_lastSelectionWasAddMode = false;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);

    Qt::KeyboardModifiers mods = event->modifiers();
    if (mods & Qt::ShiftModifier) {
        // Show Add
        if (m_selectAddButton) m_selectAddButton->setChecked(true);
        if (m_selectReplaceButton) m_selectReplaceButton->setChecked(false);
        if (m_selectSubtractButton) m_selectSubtractButton->setChecked(false);
        if (m_blurredImageCanvas) { m_blurredImageCanvas->setReplaceMode(false); m_blurredImageCanvas->setAddMode(true); }
    } else if (mods & Qt::ControlModifier) {
        // Show Subtract
        if (m_selectSubtractButton) m_selectSubtractButton->setChecked(true);
        if (m_selectReplaceButton) m_selectReplaceButton->setChecked(false);
        if (m_selectAddButton) m_selectAddButton->setChecked(false);
        if (m_blurredImageCanvas) { m_blurredImageCanvas->setReplaceMode(false); m_blurredImageCanvas->setAddMode(false); }
    } else {
        // Replace
        if (m_selectReplaceButton) m_selectReplaceButton->setChecked(true);
        if (m_selectAddButton) m_selectAddButton->setChecked(false);
        if (m_selectSubtractButton) m_selectSubtractButton->setChecked(false);
        if (m_blurredImageCanvas) { m_blurredImageCanvas->setReplaceMode(true); m_blurredImageCanvas->setAddMode(true); }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QMainWindow::keyReleaseEvent(event);

    Qt::KeyboardModifiers mods = QGuiApplication::keyboardModifiers();
    if (mods & Qt::ShiftModifier) {
        if (m_selectAddButton) m_selectAddButton->setChecked(true);
    } else if (mods & Qt::ControlModifier) {
        if (m_selectSubtractButton) m_selectSubtractButton->setChecked(true);
    } else {
        if (m_selectReplaceButton) m_selectReplaceButton->setChecked(true);
    }
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

void MainWindow::onUndoClicked()
{
    m_session.undo();
    m_blurredPixmap = m_session.blurredPixmap();
    updatePreviewLabels();
}

void MainWindow::onRedoClicked()
{
    m_session.redo();
    m_blurredPixmap = m_session.blurredPixmap();
    updatePreviewLabels();
}

void MainWindow::onSelectionChanged(const QImage &mask)
{
    if (!m_session.hasImage())
        return;

    // Run selection-based blur/remove in background to avoid UI blocking
    int strength = m_blurSlider->value();
    m_lastBlurJobStrength = strength;

    if (m_lastSelectionWasAddMode) {
        // Save undo state for manual selection operation
        m_session.pushState();
        QFuture<QPixmap> fut = QtConcurrent::run([sessionCopy = m_session, strength, mask]() mutable -> QPixmap {
            sessionCopy.applyFakeBlur(strength, mask);
            return sessionCopy.blurredPixmap();
        });
        m_blurWatcher->setFuture(fut);
    } else {
        // Save undo state for removal as well
        m_session.pushState();
        QFuture<QPixmap> fut = QtConcurrent::run([sessionCopy = m_session, mask]() mutable -> QPixmap {
            sessionCopy.removeBlur(mask);
            return sessionCopy.blurredPixmap();
        });
        m_blurWatcher->setFuture(fut);
    }

    // Enable slider after first blur operation
    if (!m_blurSlider->isEnabled())
        m_blurSlider->setEnabled(true);
}

void MainWindow::onSelectionModeChanged(bool addMode, bool replaceMode)
{
    m_lastSelectionWasAddMode = addMode;

    // Update UI buttons to reflect current modifier-driven mode
    if (m_selectReplaceButton && m_selectAddButton && m_selectSubtractButton) {
        if (replaceMode) {
            m_selectReplaceButton->blockSignals(true);
            m_selectAddButton->blockSignals(true);
            m_selectSubtractButton->blockSignals(true);
            m_selectReplaceButton->setChecked(true);
            m_selectAddButton->setChecked(false);
            m_selectSubtractButton->setChecked(false);
            m_selectReplaceButton->blockSignals(false);
            m_selectAddButton->blockSignals(false);
            m_selectSubtractButton->blockSignals(false);
        } else {
            // addMode true -> Add, false -> Subtract
            m_selectReplaceButton->blockSignals(true);
            m_selectAddButton->blockSignals(true);
            m_selectSubtractButton->blockSignals(true);
            m_selectReplaceButton->setChecked(false);
            m_selectAddButton->setChecked(addMode);
            m_selectSubtractButton->setChecked(!addMode);
            m_selectReplaceButton->blockSignals(false);
            m_selectAddButton->blockSignals(false);
            m_selectSubtractButton->blockSignals(false);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (!mime->hasUrls()) {
        event->ignore();
        return;
    }

    // Check if there is at least one image file
    bool hasImage = false;
    for (const QUrl &url : mime->urls()) {
        const QString path = url.toLocalFile().toLower();
        if (path.endsWith(".png") ||
            path.endsWith(".jpg") ||
            path.endsWith(".jpeg") ||
            path.endsWith(".bmp") ||
            path.endsWith(".webp")) {
            hasImage = true;
            break;
        }
    }

    if (hasImage)
        event->acceptProposedAction();
    else
        event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (!mime->hasUrls()) {
        event->ignore();
        return;
    }

    QString imagePath;
    for (const QUrl &url : mime->urls()) {
        const QString path = url.toLocalFile();
        const QString lower = path.toLower();
        if (lower.endsWith(".png") ||
            lower.endsWith(".jpg") ||
            lower.endsWith(".jpeg") ||
            lower.endsWith(".bmp") ||
            lower.endsWith(".webp")) {
            imagePath = path;
            break;
        }
    }

    if (imagePath.isEmpty()) {
        event->ignore();
        return;
    }

    // Reuse the same logic as the Upload button
    // Factor your current upload code into a helper, e.g. loadImageFromPath(...)
    if (!m_session.loadImage(imagePath)) {
        QMessageBox::warning(this, "Error", "Could not load the dropped image.");
        event->ignore();
        return;
    }

    m_currentImagePath = imagePath;
    m_originalPixmap   = m_session.originalPixmap();
    m_blurredPixmap    = m_session.blurredPixmap();
    // Clear selection because this is a newly loaded image from drop
    if (m_blurredImageCanvas) m_blurredImageCanvas->clearSelection();
    updatePreviewLabels();
    m_infoLabel->setText(QFileInfo(imagePath).fileName());
    m_pages->setCurrentWidget(m_previewPage);

    event->acceptProposedAction();
}
