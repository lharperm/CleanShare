#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QRect>
#include <QVector>
#include <QFutureWatcher>

#include "SessionController.h"

class QStackedWidget;
class QWidget;
class QLabel;
class QPushButton;
class QToolButton;
class QSlider;
class QSpinBox;
class QTimer;
class QDragEnterEvent;
class QDropEvent;
class QKeyEvent;
class QImage;

class ImageCanvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    // Home page
    void onUploadClicked();

    // Detection & blur
    void onDetectClicked();
    void onDetectionsUpdated(const QVector<QRect> &boxes);

    void onBlurSliderChanged(int value);
    void onBlurSpinChanged(int value);
    void onBlurDebounceTimeout();
    void onBackgroundBlurFinished();

    // Selection mode buttons
    void onSelectReplaceClicked(bool checked);
    void onSelectAddClicked(bool checked);
    void onSelectSubtractClicked(bool checked);

    // Export & manual edit
    void onExportClicked();
    void onManualEditClicked();
    void onUndoClicked();
    void onRedoClicked();

    // Manual selection from canvas
    void onSelectionChanged(const QImage &mask);
    void onSelectionModeChanged(bool addMode, bool replaceMode);

private:
    void createHomePage();
    void createPreviewPage();
    void showImageInPanels();
    void updatePreviewLabels();
    void applyFakeBlur(int strength);

    // Pages
    QStackedWidget *m_pages = nullptr;
    QWidget *m_homePage = nullptr;
    QPushButton *m_uploadButton = nullptr;
    QLabel *m_infoLabel = nullptr;
    QWidget *m_previewPage = nullptr;

    // Preview widgets
    QLabel *m_originalImageLabel = nullptr;
    ImageCanvas *m_blurredImageCanvas = nullptr;

    // Toolbar controls
    QPushButton *m_detectButton = nullptr;
    QPushButton *m_manualEditButton = nullptr;
    QPushButton *m_exportButton = nullptr;
    QPushButton *m_undoButton = nullptr;
    QPushButton *m_redoButton = nullptr;

    QToolButton *m_selectReplaceButton = nullptr;
    QToolButton *m_selectAddButton = nullptr;
    QToolButton *m_selectSubtractButton = nullptr;

    QSpinBox *m_blurSpinBox = nullptr;
    QSlider *m_blurSlider = nullptr;
    QLabel *m_blurValueLabel = nullptr;
    QTimer *m_blurDebounceTimer = nullptr;
    QFutureWatcher<QPixmap> *m_blurWatcher = nullptr;

    int  m_pendingBlurValue = 50;
    bool m_lastSelectionWasAddMode = true;
    int  m_lastBlurJobStrength = -1;
    bool m_manualEditEnabled = false;

    // Current image
    QString m_currentImagePath;
    QPixmap m_originalPixmap;
    QPixmap m_blurredPixmap;

    // Core session
    SessionController m_session;
};

#endif // MAINWINDOW_H