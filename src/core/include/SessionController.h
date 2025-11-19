#pragma once

#include <QObject>
#include <QImage>

class SessionController : public QObject {
    Q_OBJECT
public:
    explicit SessionController(QObject* parent = nullptr);

public slots:
    void handleImportImage();                    // existing (file dialog)
    void handleImportImageFromPath(const QString& path); // NEW
    void handleRunDetection();
    void handleExportImage();

    signals:
        void imageUpdated(const QImage& original, const QImage& redacted);
    void statusMessage(const QString& msg);

private:
    QImage m_original;
    QImage m_redacted;

    void loadImageFromPath(const QString& path); // NEW helper
};
