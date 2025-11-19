#pragma once

#include <QWidget>
#include <QImage>

class QLabel;
class QPushButton;
class QFrame;

class ImagePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);
    void setImages(const QImage& original, const QImage& redacted);

signals:
    void autoDetectClicked();           // central button
    void imageFileDropped(const QString& path); // drag & drop

protected:
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QFrame*      m_leftFrame  = nullptr;
    QFrame*      m_rightFrame = nullptr;
    QLabel*      m_leftLabel  = nullptr;
    QLabel*      m_rightLabel = nullptr;
    QPushButton* m_autoDetectButton = nullptr;

    QImage m_leftImg;
    QImage m_rightImg;

    void updateLabels();
};
