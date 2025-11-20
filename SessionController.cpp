#include "SessionController.h"

#include <QImage>
#include <QMessageBox>  // not actually used here, just in case

SessionController::SessionController()
    : m_currentImagePath()
    , m_original()
    , m_blurred()
{
}

bool SessionController::loadImage(const QString &filePath)
{
    QPixmap pix(filePath);
    if (pix.isNull()) {
        return false;
    }

    m_currentImagePath = filePath;
    m_original = pix;
    m_blurred  = pix;
    return true;
}

void SessionController::applyFakeBlur(int strength)
{
    if (m_original.isNull())
        return;

    // Stronger pixelation: map [0,100] to factor [1, 80]
    int minFactor = 1;
    int maxFactor = 80;
    int factor = minFactor + (maxFactor - minFactor) * strength / 100;

    if (factor <= 1) {
        m_blurred = m_original;
        return;
    }

    QImage src = m_original.toImage();
    QSize smallSize(src.width() / factor, src.height() / factor);

    if (smallSize.width() < 1 || smallSize.height() < 1) {
        m_blurred = m_original;
        return;
    }

    QImage small = src.scaled(
        smallSize,
        Qt::IgnoreAspectRatio,
        Qt::FastTransformation
        );

    QImage blurred = small.scaled(
        src.size(),
        Qt::IgnoreAspectRatio,
        Qt::FastTransformation
        );

    m_blurred = QPixmap::fromImage(blurred);
}

