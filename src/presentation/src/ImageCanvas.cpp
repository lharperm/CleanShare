#include "ImageCanvas.h"

#include <QPainter>
#include <QMouseEvent>
#include <QStyleOption>

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent)
    , m_editingEnabled(false)
    , m_drawing(false)
    , m_frameStyle(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void ImageCanvas::setImage(const QPixmap &pixmap)
{
    m_originalImage = pixmap;
    updateScaledImage();
    update();
}

void ImageCanvas::setEditingEnabled(bool enabled)
{
    m_editingEnabled = enabled;
    // You could change cursor here if you want
    setCursor(m_editingEnabled ? Qt::CrossCursor : Qt::ArrowCursor);
}

void ImageCanvas::updateScaledImage()
{
    if (m_originalImage.isNull() || width() <= 0 || height() <= 0) {
        m_scaledImage = QPixmap();
        return;
    }

    m_scaledImage = m_originalImage.scaled(
        size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );
}

void ImageCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    // Draw the scaled image centered
    if (!m_scaledImage.isNull()) {
        QSize imgSize = m_scaledImage.size();
        QPoint topLeft((width() - imgSize.width()) / 2,
                       (height() - imgSize.height()) / 2);
        p.drawPixmap(topLeft, m_scaledImage);
    }

    // Draw brush strokes (overlays)
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);

    for (const QPainterPath &path : m_paths) {
        p.drawPath(path);
    }
    if (m_drawing) {
        p.drawPath(m_currentPath);
    }

    // Optional simple frame (like QLabel::setFrameStyle)
    if (m_frameStyle != 0) {
        p.setPen(Qt::gray);
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
}

void ImageCanvas::mousePressEvent(QMouseEvent *event)
{
    if (!m_editingEnabled || m_scaledImage.isNull())
        return;

    if (event->button() == Qt::LeftButton) {
        m_drawing = true;
        m_currentPath = QPainterPath(event->pos());
        update();
    }
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_editingEnabled || !m_drawing)
        return;

    m_currentPath.lineTo(event->pos());
    update();
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_editingEnabled || !m_drawing)
        return;

    if (event->button() == Qt::LeftButton) {
        m_drawing = false;
        m_paths.append(m_currentPath);
        m_currentPath = QPainterPath();
        update();
    }
}

void ImageCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaledImage();
}
