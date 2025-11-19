#pragma once

#include <QMainWindow>

class ImagePreviewWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    signals:
        void importImageRequested();
    void runDetectionRequested();
    void exportImageRequested();

public slots:
    void onImageUpdated(const QImage& original, const QImage& redactedPreview);
    void showStatusMessage(const QString& msg);

private slots:
    void handleImportTriggered();
    void handleDetectTriggered();
    void handleExportTriggered();

private:
    void createActions();
    void createToolbar();
    void createCentralPane();

    ImagePreviewWidget* m_previewWidget = nullptr;
    QAction* m_importAction = nullptr;
    QAction* m_detectAction = nullptr;
    QAction* m_exportAction = nullptr;
};
