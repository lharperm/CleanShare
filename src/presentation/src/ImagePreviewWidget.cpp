#include "ImagePreviewWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);

    m_leftLabel = new QLabel("Original", this);
    m_rightLabel = new QLabel("Blurred", this);

    m_leftLabel->setAlignment(Qt::AlignCenter);
    m_rightLabel->setAlignment(Qt::AlignCenter);

    m_leftLabel->setMinimumSize(200, 200);
    m_rightLabel->setMinimumSize(200, 200);

    layout->addWidget(m_leftLabel);
    layout->addWidget(m_rightLabel);

    setLayout(layout);
}

void ImagePreviewWidget::setImages(const QImage& original,
                                   const QImage& redacted)
{
    m_leftImg = original;
    m_rightImg = redacted;
    updateLabels();
}

void ImagePreviewWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateLabels();
}

void ImagePreviewWidget::updateLabels() {
    if (!m_leftImg.isNull()) {
        QPixmap scaledLeft = QPixmap::fromImage(m_leftImg)
                                 .scaled(m_leftLabel->size(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
        m_leftLabel->setPixmap(scaledLeft);
    } else {
        m_leftLabel->setPixmap(QPixmap());
    }

    if (!m_rightImg.isNull()) {
        QPixmap scaledRight = QPixmap::fromImage(m_rightImg)
                                  .scaled(m_rightLabel->size(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
        m_rightLabel->setPixmap(scaledRight);
    } else {
        m_rightLabel->setPixmap(QPixmap());
    }
}
