#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "SessionController.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QString>
#include <QPixmap>

class ImageCanvas;  // forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onUploadClicked();
    void onDetectClicked();
    void onBlurSliderChanged(int);
    void onExportClicked();
    void onManualEditClicked();
    void onUndoClicked();
    void onRedoClicked();

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
    QSlider     *m_blurSlider;

    QString m_currentImagePath;
    QPixmap m_originalPixmap;
    QPixmap m_blurredPixmap;

    SessionController m_session;

    bool m_manualEditEnabled;
};

#endif // MAINWINDOW_H
