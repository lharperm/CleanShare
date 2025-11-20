#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include <QString>
#include <QPixmap>

class SessionController
{
public:
    SessionController();

    bool loadImage(const QString &filePath);

    // Applies fake blur based on strength [0, 100]
    void applyFakeBlur(int strength);

    const QPixmap &originalPixmap() const { return m_original; }
    const QPixmap &blurredPixmap()  const { return m_blurred; }

    bool hasImage() const { return !m_original.isNull(); }

    const QString &currentImagePath() const { return m_currentImagePath; }

private:
    QString m_currentImagePath;
    QPixmap m_original;
    QPixmap m_blurred;
};

#endif // SESSIONCONTROLLER_H
