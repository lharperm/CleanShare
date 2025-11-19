#include "SessionController.h"

#include <QFileDialog>
#include <QStandardPaths>

SessionController::SessionController(QObject* parent)
    : QObject(parent)
{
}

void SessionController::loadImageFromPath(const QString& path)
{
    if (path.isEmpty()) {
        emit statusMessage("Import cancelled");
        return;
    }

    QImage img(path);
    if (img.isNull()) {
        emit statusMessage("Failed to load image");
        return;
    }

    m_original = img;
    m_redacted = QImage();  // nothing redacted yet

    emit statusMessage("Image loaded");
    emit imageUpdated(m_original, m_redacted);
}

void SessionController::handleImportImage()
{
    const QString picturesDir =
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    const QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Import image",
        picturesDir,
        "Images (*.png *.jpg *.jpeg)"
    );

    loadImageFromPath(path);
}

void SessionController::handleImportImageFromPath(const QString& path)
{
    loadImageFromPath(path);
}

void SessionController::handleRunDetection()
{
    if (m_original.isNull()) {
        emit statusMessage("No image loaded");
        return;
    }

    // TEMP: fake detection – just copy original to "redacted"
    m_redacted = m_original;

    emit statusMessage("Detection stub: redacted = original (for now)");
    emit imageUpdated(m_original, m_redacted);
}

void SessionController::handleExportImage()
{
    if (m_redacted.isNull()) {
        emit statusMessage("Nothing to export");
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        nullptr,
        "Export image",
        {},
        "PNG (*.png);;JPEG (*.jpg *.jpeg)"
    );

    if (path.isEmpty()) {
        emit statusMessage("Export cancelled");
        return;
    }

    if (!m_redacted.save(path)) {
        emit statusMessage("Failed to save image");
        return;
    }

    emit statusMessage("Image exported");
}
