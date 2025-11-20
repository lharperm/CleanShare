#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QWidget>
#include <QPixmap>
#include <QPainterPath>
#include <QVector>

class ImageCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ImageCanvas(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);
    void setEditingEnabled(bool enabled);
    bool editingEnabled() const { return m_editingEnabled; }

    // Optional, so MainWindow can give it a frame style like a QLabel
    void setFrameStyle(int style) { m_frameStyle = style; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateScaledImage();

    QPixmap m_originalImage;
    QPixmap m_scaledImage;

    bool m_editingEnabled;
    bool m_drawing;
    QPainterPath m_currentPath;
    QVector<QPainterPath> m_paths;

    int m_frameStyle;   // for a simple box border
};

#endif // IMAGECANVAS_H
