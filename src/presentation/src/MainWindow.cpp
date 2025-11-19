#include "MainWindow.h"
#include "ImagePreviewWidget.h"

#include <QAction>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QImage>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    createActions();
    createToolbar();
    createCentralPane();

    statusBar()->showMessage("Ready");
    setWindowTitle("CleanShare");
    resize(1200, 700);
}

void MainWindow::createActions() {
    m_importAction = new QAction(tr("Import Image"), this);
    connect(m_importAction, &QAction::triggered,
            this, &MainWindow::handleImportTriggered);

    m_detectAction = new QAction(tr("Auto Detect && Blur"), this);
    connect(m_detectAction, &QAction::triggered,
            this, &MainWindow::handleDetectTriggered);

    m_exportAction = new QAction(tr("Export Image"), this);
    connect(m_exportAction, &QAction::triggered,
            this, &MainWindow::handleExportTriggered);

    // Disabled until we have an image
    m_detectAction->setEnabled(false);
    m_exportAction->setEnabled(false);
}

void MainWindow::createToolbar() {
    auto* toolbar = addToolBar(tr("Main"));
    toolbar->addAction(m_importAction);
    toolbar->addAction(m_detectAction);
    toolbar->addAction(m_exportAction);
}

void MainWindow::createCentralPane() {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    m_previewWidget = new ImagePreviewWidget(central);
	connect(m_previewWidget, &ImagePreviewWidget::autoDetectClicked,
        this, &MainWindow::handleDetectTriggered);

    connect(m_previewWidget, &ImagePreviewWidget::imageFileDropped,
        this, &MainWindow::handleImageFileDropped);

    layout->addWidget(m_previewWidget);

    layout->setContentsMargins(4, 4, 4, 4);
    central->setLayout(layout);
    setCentralWidget(central);

	connect(m_previewWidget, &ImagePreviewWidget::autoDetectClicked,this, &MainWindow::handleDetectTriggered);
}

void MainWindow::handleImportTriggered() {
    emit importImageRequested();
}

void MainWindow::handleDetectTriggered() {
    emit runDetectionRequested();
}

void MainWindow::handleExportTriggered() {
    emit exportImageRequested();
}

void MainWindow::onImageUpdated(const QImage& original,
                                const QImage& redactedPreview)
{
    m_previewWidget->setImages(original, redactedPreview);

    const bool hasOriginal = !original.isNull();
    const bool hasRedacted = !redactedPreview.isNull();

    m_detectAction->setEnabled(hasOriginal);
    m_exportAction->setEnabled(hasOriginal && hasRedacted);
}

void MainWindow::showStatusMessage(const QString& msg) {
    constexpr int DEFAULT_TIMEOUT_MS = 3000;
    statusBar()->showMessage(msg, DEFAULT_TIMEOUT_MS);
}

void MainWindow::handleImageFileDropped(const QString& path) {
    emit importImageFromPathRequested(path);
}

