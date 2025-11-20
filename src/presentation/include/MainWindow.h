#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "SessionController.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QSpinBox>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QTimer>
#include <QFutureWatcher>

class ImageCanvas;  // forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    // Track keyboard modifiers to update selection-mode buttons when user presses Shift/Ctrl
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onUploadClicked();
    void onDetectClicked();
    void onBlurSliderChanged(int);
    void onBlurDebounceTimeout();
    void onBackgroundBlurFinished();
    void onExportClicked();
    void onManualEditClicked();
    void onUndoClicked();
    void onRedoClicked();
    void onSelectionChanged(const QImage &mask);
    void onSelectionModeChanged(bool addMode, bool replaceMode);
    void onSelectReplaceClicked(bool checked);
    void onSelectAddClicked(bool checked);
    void onSelectSubtractClicked(bool checked);
    void onBlurSpinChanged(int value);

private:
    void createHomePage();
    void createPreviewPage();
    void showImageInPanels();
    void updatePreviewLabels();
    void applyFakeBlur(int strength);   // calls into SessionController

    QStackedWidget *m_pages;

    // Home page
    QWidget *m_homePage;
    QPushButton *m_uploadButton;
    QLabel *m_infoLabel;

    // Preview / Edit page
    QWidget *m_previewPage;
    QLabel *m_originalImageLabel;
    ImageCanvas *m_blurredImageCanvas;

    QPushButton *m_detectButton;
    QPushButton *m_manualEditButton;
    QPushButton *m_exportButton;
    QPushButton *m_undoButton;
    QPushButton *m_redoButton;
    QToolButton *m_selectReplaceButton;
    QToolButton *m_selectAddButton;
    QToolButton *m_selectSubtractButton;
    QSlider     *m_blurSlider;
    QSpinBox    *m_blurSpinBox;
    QTimer      *m_blurDebounceTimer;
    int         m_pendingBlurValue;
    bool        m_lastSelectionWasAddMode;
    QLabel      *m_blurValueLabel;
    QFutureWatcher<QPixmap> *m_blurWatcher;
    int m_lastBlurJobStrength;

    QString m_currentImagePath;
    QPixmap m_originalPixmap;
    QPixmap m_blurredPixmap;

    SessionController m_session;

    bool m_manualEditEnabled;
};

#endif // MAINWINDOW_H
