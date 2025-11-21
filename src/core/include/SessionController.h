#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include <QString>
#include <QPixmap>

class SessionController
{
public:
    SessionController();
    bool autoBlurWithPythonDetections(int strength, QString *errorMessage = nullptr);
    bool loadImage(const QString &filePath);

    // Applies fake blur based on strength [0, 100]
    void applyFakeBlur(int strength);

    const QPixmap &originalPixmap() const { return m_original; }
    const QPixmap &blurredPixmap()  const { return m_blurred; }

    bool hasImage() const { return !m_original.isNull(); }

    const QString &currentImagePath() const { return m_currentImagePath; }

    void undo();
    void redo();
    void pushState();

    

private:
    QString m_currentImagePath;
    QPixmap m_original;
    QPixmap m_blurred;

    QVector<QPixmap> m_undoStack;
    QVector<QPixmap> m_redoStack;
};

#endif // SESSIONCONTROLLER_H
