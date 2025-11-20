#include "ImageCanvas.h"

#include <QPainter>
#include <QMouseEvent>
#include <QStyleOption>

ImageCanvas::ImageCanvas(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void ImageCanvas::setImage(const QPixmap &pixmap)
{
    // If the new pixmap has a different size than the current one, clear selection
    bool sizeChanged = (m_originalImage.size() != pixmap.size());
    m_originalImage = pixmap;
    updateScaledImage();
    if (sizeChanged) {
        m_selectionPath = QPainterPath();
        m_currentPath   = QPainterPath();
    }
    update();
}

void ImageCanvas::clearSelection()
{
    m_selectionPath = QPainterPath();
    m_currentPath = QPainterPath();
    update();
}

void ImageCanvas::setEditingEnabled(bool enabled)
{
    m_editingEnabled = enabled;
    update();
}

void ImageCanvas::updateScaledImage()
{
    if (m_originalImage.isNull()) {
        m_scaledImage = QPixmap();
        m_targetRect = QRectF();
        return;
    }

    QSize availSize = size();
    QSize imgSize   = m_originalImage.size();
    QSize scaledSize = imgSize;
    scaledSize.scale(availSize, Qt::KeepAspectRatio);

    m_scaledImage = m_originalImage.scaled(scaledSize,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);

    qreal x = (width()  - scaledSize.width())  / 2.0;
    qreal y = (height() - scaledSize.height()) / 2.0;
    m_targetRect = QRectF(QPointF(x, y), scaledSize);
}

QRectF ImageCanvas::imageTargetRect() const
{
    return m_targetRect;
}

QPointF ImageCanvas::widgetToImage(const QPointF &p) const
{
    if (m_originalImage.isNull() || m_targetRect.isEmpty())
        return QPointF();

    qreal sx = (p.x() - m_targetRect.left())  / m_targetRect.width();
    qreal sy = (p.y() - m_targetRect.top())   / m_targetRect.height();

    sx = qBound(0.0, sx, 1.0);
    sy = qBound(0.0, sy, 1.0);

    qreal ix = sx * m_originalImage.width();
    qreal iy = sy * m_originalImage.height();

    return QPointF(ix, iy);
}

void ImageCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.fillRect(rect(), palette().window());

    if (m_scaledImage.isNull())
        return;

    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.drawPixmap(m_targetRect.toRect(), m_scaledImage);

    if (!m_selectionPath.isEmpty() || !m_currentPath.isEmpty()) {
        p.save();

        // Transform painter to image coordinates for filling paths
        QTransform t;
        t.translate(m_targetRect.left(), m_targetRect.top());
        t.scale(m_targetRect.width() / m_originalImage.width(),
                m_targetRect.height() / m_originalImage.height());
        p.setTransform(t, true);
        p.setRenderHint(QPainter::Antialiasing, true);

        QColor highlight(0, 120, 215, 80);   // translucent blue
        QPainterPath toDraw = m_selectionPath;
        if (!m_currentPath.isEmpty())
            toDraw.addPath(m_currentPath);

        p.fillPath(toDraw, highlight);
        // Draw dashed outline like Photoshop
        QPen outlinePen(QColor(0, 120, 215), 2, Qt::DashLine);
        outlinePen.setCosmetic(true);
        p.setPen(outlinePen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(toDraw);
        p.restore();
    }

    // Optional focus border
    if (m_editingEnabled) {
        p.setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
        p.drawRect(m_targetRect.adjusted(-2, -2, 2, 2));
    }
}

void ImageCanvas::mousePressEvent(QMouseEvent *event)
{
    if (!m_editingEnabled || m_originalImage.isNull())
        return;

    if (event->button() != Qt::LeftButton)
        return;

    if (!m_targetRect.contains(event->pos()))
        return;

    m_drawing = true;

    Qt::KeyboardModifiers mods = event->modifiers();
    if (mods & Qt::ControlModifier) {
        m_addMode = false;
        m_replaceMode = false;
    } else if (mods & Qt::ShiftModifier) {
        m_addMode = true;
        m_replaceMode = false;
    } else {
        // no modifier: replace
        m_addMode = true;
        m_replaceMode = true;
    }

    QPointF imgPos = widgetToImage(event->pos());
    m_currentPath = QPainterPath(imgPos);
    // Notify listeners about selection mode so UI buttons can reflect modifier keys immediately
    emit selectionModeChanged(m_addMode, m_replaceMode);
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawing)
        return;
    if (!m_targetRect.contains(event->pos()))
        return;

    QPointF imgPos = widgetToImage(event->pos());
    m_currentPath.lineTo(imgPos);
    update();
}

void ImageCanvas::setAddMode(bool add)
{
    m_addMode = add;
}

void ImageCanvas::setReplaceMode(bool replace)
{
    m_replaceMode = replace;
}

void ImageCanvas::applyStrokeToSelection(bool replace, bool addMode)
{
    if (m_currentPath.isEmpty())
        return;

    QPainterPath strokePath;
    strokePath.addPath(m_currentPath);
    strokePath = strokePath.simplified();

    if (replace) {
        m_selectionPath = strokePath;
    } else {
        if (addMode) {
            m_selectionPath = m_selectionPath.united(strokePath);
        } else {
            m_selectionPath = m_selectionPath.subtracted(strokePath);
        }
    }

    m_currentPath = QPainterPath();
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_drawing || event->button() != Qt::LeftButton)
        return;

    m_drawing = false;

    applyStrokeToSelection(m_replaceMode, m_addMode);
    update();

    // Emit updated mask for interested listeners (MainWindow / SessionController)
    QImage mask = selectionMask();
    emit selectionChanged(mask);
    emit selectionModeChanged(m_addMode, m_replaceMode);
}

void ImageCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateScaledImage();
    update();
}

QImage ImageCanvas::selectionMask() const
{
    if (m_originalImage.isNull() || m_selectionPath.isEmpty())
        return QImage();

    QImage mask(m_originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);

    QPainter p(&mask);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);  // white = selected
    p.fillPath(m_selectionPath, Qt::white);

    return mask;
}
