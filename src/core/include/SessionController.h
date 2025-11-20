#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include <QString>
#include <QPixmap>
#include <QImage>
#include <QVector>

class SessionController
{
public:
    SessionController();

    bool loadImage(const QString &filePath);

    // Applies fake blur based on strength [0, 100]
    void applyFakeBlur(int strength);                  // whole image
    void applyFakeBlur(int strength, const QImage &mask); // selection only
    void removeBlur(const QImage &mask);               // remove blur from mask area

    const QPixmap &originalPixmap() const { return m_original; }
    const QPixmap &blurredPixmap()  const { return m_blurred; }

    bool hasImage() const { return !m_original.isNull(); }

    const QString &currentImagePath() const { return m_currentImagePath; }

    void undo();
    void redo();
    void pushState();

    // Get current blur mask (accumulated from all mask-based blurs)
    const QImage &currentBlurMask() const { return m_cumulativeBlurMask; }
    void clearBlurMask() { m_cumulativeBlurMask = QImage(); }
    
    // Adopt a computed full-size blur (used by background worker to commit result)
    void adoptComputedFullBlur(const QPixmap &pixmap, int strength);

private:
    QString m_currentImagePath;
    QPixmap m_original;
    QPixmap m_blurred;
    QImage  m_cumulativeBlurMask;  // tracks which areas are blurred

    // Cache for full-image blur at different strengths (avoid recomputing)
    int         m_cachedBlurStrength;
    QPixmap     m_cachedBlurredImage;

    QVector<QPixmap> m_undoStack;
    QVector<QPixmap> m_redoStack;
};

#endif // SESSIONCONTROLLER_H
