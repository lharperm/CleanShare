#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QVector>
#include <QRect>
#include <vector>

class SessionController : public QObject
{
    Q_OBJECT   // <-- THIS, not "QObject"

public:
    explicit SessionController(QObject *parent = nullptr);

    bool loadImage(const QString &filePath);
    bool autoBlurWithPythonDetections(int strength, QString *errorMessage = nullptr);

    bool runDetection();          // (fine to leave for later, even if unused)
    void applyBlur(int strength); // (same)
    void applyFakeBlur(int strength);
    void applyFakeBlur(int strength, const QImage &mask);
    void adoptComputedFullBlur(const QPixmap &pixmap, int strength);
    void removeBlur(const QImage &mask);

    const QPixmap &originalPixmap() const { return m_original; }
    const QPixmap &blurredPixmap()  const { return m_blurred; }

    bool hasImage() const { return !m_original.isNull(); }
    const QString &currentImagePath() const { return m_currentImagePath; }
    const QImage &cumulativeMask() const { return m_cumulativeBlurMask; }
    void undo();
    void redo();
    void pushState();

signals:
    void imagesUpdated(const QPixmap &before, const QPixmap &after);
    void detectionsUpdated(const QVector<QRect> &boxes);

private:
    QString m_currentImagePath;
    QPixmap m_original;
    QPixmap m_blurred;
    QImage  m_cumulativeBlurMask;      // union of auto + manual
    bool    m_hasDetectionMask = false;
    QVector<QRect> m_autoBoxes;

    int     m_cachedBlurStrength = -1;
    QPixmap m_cachedBlurredImage;

    QVector<QPixmap> m_undoStack;
    QVector<QPixmap> m_redoStack;
};

#endif // SESSIONCONTROLLER_H
