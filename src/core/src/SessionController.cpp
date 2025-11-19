#include "SessionController.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>

SessionController::SessionController(QObject* parent)
    : QObject(parent)
{
}

void SessionController::loadImageFromPath(const QString& path) {
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
    m_redacted = QImage();

    emit statusMessage("Image loaded");
    emit imageUpdated(m_original, m_redacted);
}

void SessionController::handleImportImage() {
    const QString picturesDir =
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    QString path = QFileDialog::getOpenFileName(
        nullptr,
        "Import image",
        picturesDir,
        "Images (*.png *.jpg *.jpeg)"
    );

    loadImageFromPath(path);
}

void SessionController::handleImportImageFromPath(const QString& path) {
    loadImageFromPath(path);
}
