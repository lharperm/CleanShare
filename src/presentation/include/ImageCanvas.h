#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>
#include <QVector>
#include <QRect>
#include <QImage>

class ImageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);
    void setEditingEnabled(bool enabled);
    bool editingEnabled() const { return m_editingEnabled; }

    // Selection API
    QImage selectionMask() const;   // mask in ORIGINAL image resolution
    void clearSelection();

    // Frame border (optional)
    void setFrameStyle(int style) { m_frameStyle = style; update(); }

    // Detection rectangles from model (original-image coordinates)
    void setDetectionBoxes(const QVector<QRect> &boxes);
    void clearDetectionBoxes();

    // Selection mode (controlled by MainWindow buttons / modifiers)
    void setReplaceMode(bool on) { m_replaceMode = on; }
    void setAddMode(bool on)     { m_addMode = on; }
    void setExistingMask(const QImage &mask);
signals:
    void selectionChanged(const QImage &mask);
    void selectionModeChanged(bool addMode, bool replaceMode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateScaledImage();
    QRect scaledImageRect() const;

    QPixmap m_originalImage;
    QPixmap m_scaledImage;

    bool m_editingEnabled;
    bool m_drawing;
    bool m_replaceMode = true;
    bool m_addMode     = true;

    QPainterPath m_currentPath;
    QVector<QPainterPath> m_paths;
    QVector<QRect> m_detectionBoxes;
    QImage m_selectionMask;   // manual paint
    QImage m_existingMask;    // auto+manual cumulative from SessionController
    int m_frameStyle;
};

#endif // IMAGECANVAS_H
