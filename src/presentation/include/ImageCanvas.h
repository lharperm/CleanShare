#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>

class ImageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);
    void clearSelection();
    void setEditingEnabled(bool enabled);
    void setAddMode(bool add);
    void setReplaceMode(bool replace);

    // Returns a mask in image coordinates (same size as original image).
    QImage selectionMask() const;

signals:
    void selectionChanged(const QImage &mask);
    // Emits (addMode, replaceMode): addMode=true means union operation when replaceMode==false.
    // replaceMode=true means the stroke replaces the current selection.
    void selectionModeChanged(bool addMode, bool replaceMode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateScaledImage();
    QRectF imageTargetRect() const;           // where the image is drawn in widget coords
    QPointF widgetToImage(const QPointF &p) const;
    void applyStrokeToSelection(bool replace, bool addMode);

    QPixmap      m_originalImage;
    QPixmap      m_scaledImage;
    QRectF       m_targetRect;               // widget rect where scaled image lives

    bool         m_editingEnabled = false;
    bool         m_drawing = false;
    bool         m_addMode = true;           // true = add, false = subtract
    bool         m_replaceMode = true;       // true when no modifier

    QPainterPath m_currentPath;              // current stroke, in image coords
    QPainterPath m_selectionPath;            // total selection, in image coords

    qreal        m_brushRadius = 30.0;       // adjust to taste
};

#endif // IMAGECANVAS_H
