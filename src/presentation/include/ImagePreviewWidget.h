#pragma once

#include <QWidget>
#include <QImage>

class QLabel;

class ImagePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);

    void setImages(const QImage& original, const QImage& redacted);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* m_leftLabel = nullptr;
    QLabel* m_rightLabel = nullptr;

    QImage m_leftImg;
    QImage m_rightImg;

    void updateLabels();
};
