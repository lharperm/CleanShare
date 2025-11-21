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
    setCursor(m_editingEnabled ? Qt::CrossCursor : Qt::ArrowCursor);
}

void ImageCanvas::setDetectionBoxes(const QVector<QRect> &boxes)
{
    m_detectionBoxes = boxes;
    update();
}

void ImageCanvas::clearDetectionBoxes()
{
    m_detectionBoxes.clear();
    update();
}

QRect ImageCanvas::scaledImageRect() const
{
    if (m_scaledImage.isNull())
        return QRect();

    QSize imgSize = m_scaledImage.size();
    QPoint topLeft((width() - imgSize.width()) / 2,
                   (height() - imgSize.height()) / 2);
    return QRect(topLeft, imgSize);
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

    QRect imgRect = scaledImageRect();

    // Draw the scaled image centered
    if (!m_scaledImage.isNull()) {
        p.drawPixmap(imgRect.topLeft(), m_scaledImage);
    }

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Draw detection rectangles (yellow) in widget coordinates
    if (!m_detectionBoxes.isEmpty() &&
        !m_originalImage.isNull() &&
        !m_scaledImage.isNull()) {git

        double sx = imgRect.width()  / static_cast<double>(m_originalImage.width());
        double sy = imgRect.height() / static_cast<double>(m_originalImage.height());

        QPen boxPen(Qt::yellow);
        boxPen.setWidth(2);
        p.setPen(boxPen);
        p.setBrush(Qt::NoBrush);

        for (const QRect &rOrig : m_detectionBoxes) {
            QRectF r(
                imgRect.left() + rOrig.left() * sx,
                imgRect.top()  + rOrig.top()  * sy,
                rOrig.width()  * sx,
                rOrig.height() * sy
            );
            p.drawRect(r);
        }
    }
    if (!m_paths.isEmpty() || m_drawing) {
        QPen selPen(QColor(0, 255, 0, 200));  // light green, semi-transparent
        selPen.setWidth(2);
        selPen.setCapStyle(Qt::RoundCap);
        selPen.setJoinStyle(Qt::RoundJoin);
        p.setPen(selPen);
        p.setBrush(Qt::NoBrush);

        for (const QPainterPath &path : m_paths) {
            p.drawPath(path);
        }
        if (m_drawing) {
            p.drawPath(m_currentPath);
        }
    }
    // Optional simple frame
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

        // Replace mode wipes previous strokes
        if (m_replaceMode) {
            m_paths.clear();
        }

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

        QImage mask = selectionMask();
        qDebug() << "[ImageCanvas] selection mask size =" << mask.size()
                 << "isNull?" << mask.isNull();

        emit selectionChanged(mask);
    }
}


void ImageCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaledImage();
}

QImage ImageCanvas::selectionMask() const
{
    if (m_originalImage.isNull() || m_paths.isEmpty())
        return QImage(); // no selection

    QImage mask(m_originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    QRect imgRect = scaledImageRect();
    if (m_scaledImage.isNull() || !imgRect.isValid()) {
        // Fallback: if scaling info is missing, draw in widget coords
        for (const QPainterPath &path : m_paths) {
            painter.drawPath(path);
        }
    }  else {
        double sx = m_originalImage.width()  / static_cast<double>(imgRect.width());
        double sy = m_originalImage.height() / static_cast<double>(imgRect.height());

        // Explicit mapping:
        // x' = sx * (x - imgRect.left())
        // y' = sy * (y - imgRect.top())
        QTransform t(
            sx, 0.0, -imgRect.left() * sx,
            0.0, sy, -imgRect.top()  * sy,
            0.0, 0.0, 1.0
        );

        QPainterPath combined;
        for (const QPainterPath &path : m_paths) {
            combined.addPath(t.map(path));
        }

        painter.drawPath(combined);
    }

    painter.end();
    return mask;
}

void ImageCanvas::clearSelection()
{
    m_paths.clear();
    m_currentPath = QPainterPath();
    update();

    // Empty selection -> emit null mask
    emit selectionChanged(QImage());
}

void ImageCanvas::setExistingMask(const QImage &mask)
{
    m_existingMask = mask;
    update();  // trigger repaint so you see it
}