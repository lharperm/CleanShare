#include "ImagePreviewWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QPushButton>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setAcceptDrops(true);  // enable drag & drop

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // --- Left rectangle (original) ---
    m_leftFrame = new QFrame(this);
    m_leftFrame->setFrameShape(QFrame::Box);
    m_leftFrame->setLineWidth(1);

    auto* leftLayout = new QVBoxLayout(m_leftFrame);
    leftLayout->setContentsMargins(4, 4, 4, 4);

    m_leftLabel = new QLabel("Drop or import image", m_leftFrame);
    m_leftLabel->setAlignment(Qt::AlignCenter);
    m_leftLabel->setMinimumSize(200, 200);
    leftLayout->addWidget(m_leftLabel);

    m_leftFrame->setLayout(leftLayout);

    // --- Center: Auto Detect button ---
    auto* centerLayout = new QVBoxLayout();
    centerLayout->addStretch();

    m_autoDetectButton = new QPushButton("Auto Detect", this);
    m_autoDetectButton->setMinimumWidth(120);
    m_autoDetectButton->setMinimumHeight(40);
    centerLayout->addWidget(m_autoDetectButton, 0, Qt::AlignCenter);

    centerLayout->addStretch();

    // --- Right rectangle (redacted) ---
    m_rightFrame = new QFrame(this);
    m_rightFrame->setFrameShape(QFrame::Box);
    m_rightFrame->setLineWidth(1);

    auto* rightLayout = new QVBoxLayout(m_rightFrame);
    rightLayout->setContentsMargins(4, 4, 4, 4);

    m_rightLabel = new QLabel("Redacted Preview", m_rightFrame);
    m_rightLabel->setAlignment(Qt::AlignCenter);
    m_rightLabel->setMinimumSize(200, 200);
    rightLayout->addWidget(m_rightLabel);

    m_rightFrame->setLayout(rightLayout);

    // Assemble layout
    mainLayout->addWidget(m_leftFrame, 1);
    mainLayout->addLayout(centerLayout);
    mainLayout->addWidget(m_rightFrame, 1);
    setLayout(mainLayout);

    // Center button fires autoDetectClicked()
    connect(m_autoDetectButton, &QPushButton::clicked,
            this, &ImagePreviewWidget::autoDetectClicked);
}

void ImagePreviewWidget::setImages(const QImage& original,
                                   const QImage& redacted)
{
    m_leftImg  = original;
    m_rightImg = redacted;
    updateLabels();

    if (m_leftImg.isNull()) {
        m_leftLabel->setText("Drop or import image");
    } else {
        m_leftLabel->setText(QString());
    }

    if (m_rightImg.isNull()) {
        m_rightLabel->setText("Redacted Preview");
    } else {
        m_rightLabel->setText(QString());
    }
}

void ImagePreviewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateLabels();
}

void ImagePreviewWidget::updateLabels()
{
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

void ImagePreviewWidget::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (!mime->hasUrls()) {
        event->ignore();
        return;
    }

    bool hasImageFile = false;
    const auto urls = mime->urls();
    for (const QUrl& url : urls) {
        if (!url.isLocalFile())
            continue;

        const QString path = url.toLocalFile().toLower();
        if (path.endsWith(".png") || path.endsWith(".jpg") || path.endsWith(".jpeg")) {
            hasImageFile = true;
            break;
        }
    }

    if (hasImageFile) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void ImagePreviewWidget::dropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    if (!mime->hasUrls()) {
        event->ignore();
        return;
    }

    // Only accept in the left rectangle
    const QPoint pos = event->position().toPoint();   // Qt 6
    if (!m_leftFrame->geometry().contains(pos)) {
        event->ignore();
        return;
    }

    QString imagePath;
    const auto urls = mime->urls();
    for (const QUrl& url : urls) {
        if (!url.isLocalFile())
            continue;

        const QString path = url.toLocalFile().toLower();
        if (path.endsWith(".png") || path.endsWith(".jpg") || path.endsWith(".jpeg")) {
            imagePath = url.toLocalFile();
            break;
        }
    }

    if (imagePath.isEmpty()) {
        event->ignore();
        return;
    }

    emit imageFileDropped(imagePath);
    event->acceptProposedAction();
}
