#pragma once

#include <QObject>
#include <QImage>
#include <QString>

class SessionController : public QObject {
    Q_OBJECT
public:
    explicit SessionController(QObject* parent = nullptr);

public slots:
    void handleImportImage();                          // via file dialog
    void handleImportImageFromPath(const QString& path); // via drag & drop
    void handleRunDetection();
    void handleExportImage();

signals:
    void imageUpdated(const QImage& original, const QImage& redacted);
    void statusMessage(const QString& msg);

private:
    QImage m_original;
    QImage m_redacted;

    void loadImageFromPath(const QString& path);       // shared helper
};
