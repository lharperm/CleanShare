#include "SessionController.h"

#include <QImage>
#include <QtMath>


#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QDebug>

SessionController::SessionController()
    : m_currentImagePath()
    , m_original()
    , m_blurred()
    , m_cumulativeBlurMask()
    , m_cachedBlurStrength(-1)
    , m_cachedBlurredImage()
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
    m_cumulativeBlurMask = QImage();  // reset mask on new image
    m_cachedBlurStrength = -1;  // invalidate cache
    m_cachedBlurredImage = QPixmap();
    return true;
}

// Box blur helper using summed-area table (integral image)
static QImage boxBlur(const QImage &src, int radius)
{
    if (radius <= 0 || src.isNull())
        return src;

    // Work in non-premultiplied ARGB32 for simpler math
    QImage img = src.convertToFormat(QImage::Format_ARGB32);
    const int w = img.width();
    const int h = img.height();

    // Use integral images for each channel to compute box sums fast
    const int iw = w + 1;
    const int ih = h + 1;
    std::vector<uint64_t> sumR((size_t)iw * ih);
    std::vector<uint64_t> sumG((size_t)iw * ih);
    std::vector<uint64_t> sumB((size_t)iw * ih);
    std::vector<uint64_t> sumA((size_t)iw * ih);

    // Build integral tables
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(img.constScanLine(y));
        uint64_t rowR = 0, rowG = 0, rowB = 0, rowA = 0;
        for (int x = 0; x < w; ++x) {
            QRgb c = line[x];
            rowR += qRed(c);
            rowG += qGreen(c);
            rowB += qBlue(c);
            rowA += qAlpha(c);

            int idx = (y + 1) * iw + (x + 1);
            int idxAbove = y * iw + (x + 1);

            sumR[idx] = sumR[idxAbove] + rowR;
            sumG[idx] = sumG[idxAbove] + rowG;
            sumB[idx] = sumB[idxAbove] + rowB;
            sumA[idx] = sumA[idxAbove] + rowA;
        }
    }

    QImage dst(w, h, QImage::Format_ARGB32);

    for (int y = 0; y < h; ++y) {
        int y1 = qMax(0, y - radius);
        int y2 = qMin(h - 1, y + radius);
        int yy1 = y1;
        int yy2 = y2;

        for (int x = 0; x < w; ++x) {
            int x1 = qMax(0, x - radius);
            int x2 = qMin(w - 1, x + radius);

            int A_x1 = x1;
            int A_y1 = yy1;
            int A_x2 = x2;
            int A_y2 = yy2;

            // indices in integral table
            int idxA = (A_y2 + 1) * iw + (A_x2 + 1);
            int idxB = (A_y1)     * iw + (A_x2 + 1);
            int idxC = (A_y2 + 1) * iw + (A_x1);
            int idxD = (A_y1)     * iw + (A_x1);

            uint64_t totalR = sumR[idxA] - sumR[idxB] - sumR[idxC] + sumR[idxD];
            uint64_t totalG = sumG[idxA] - sumG[idxB] - sumG[idxC] + sumG[idxD];
            uint64_t totalB = sumB[idxA] - sumB[idxB] - sumB[idxC] + sumB[idxD];
            uint64_t totalA = sumA[idxA] - sumA[idxB] - sumA[idxC] + sumA[idxD];

            int count = (x2 - x1 + 1) * (y2 - y1 + 1);

            uint8_t r = static_cast<uint8_t>((totalR + count/2) / count);
            uint8_t g = static_cast<uint8_t>((totalG + count/2) / count);
            uint8_t b = static_cast<uint8_t>((totalB + count/2) / count);
            uint8_t a = static_cast<uint8_t>((totalA + count/2) / count);

            dst.setPixel(x, y, qRgba(r, g, b, a));
        }
    }

    return dst.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

// PLAY WITH THIS FUNCTION TO ADJUST BLUR STRENGTH/QUALITY! :)
void SessionController::applyFakeBlur(int strength)
{
    if (m_original.isNull())
        return;

    // Map slider [0..100] to a Gaussian radius [0..30]
    // sqrt() curve gives strong blur without huge radius
    int radius = static_cast<int>(std::sqrt(strength) * 3.0);
    if (radius > 30)
        radius = 30;

    if (radius <= 0) {
        m_blurred = m_original;
        m_cumulativeBlurMask = QImage();
        m_cachedBlurStrength = -1;
        return;
    }

    // Check cache first
    if (m_cachedBlurStrength == strength && !m_cachedBlurredImage.isNull()) {
        m_blurred = m_cachedBlurredImage;
        // Do NOT set m_cumulativeBlurMask here — cumulative mask should track user selection-based edits only.
        return;
    }

    // Compute blur and cache it
    QImage src = m_original.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage blurred = boxBlur(src, radius);
    m_blurred = QPixmap::fromImage(blurred);
    
    // Cache for next time
    m_cachedBlurStrength = strength;
    m_cachedBlurredImage = m_blurred;
}

void SessionController::applyFakeBlur(int strength, const QImage &mask)
{
    if (m_original.isNull())
        return;

    if (mask.isNull() || mask.size() != m_original.size()) {
        applyFakeBlur(strength);
        return;
    }

    // radius mapping (fast)
    int radius = static_cast<int>(std::sqrt(strength) * 3.0);
    if (radius > 30)
        radius = 30;

    if (radius <= 0) {
        m_blurred = m_original;
        m_cumulativeBlurMask = QImage();
        return;
    }

    // Start from the ORIGINAL image
    QImage base    = m_original.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage blurred = boxBlur(base, radius);
    
    // Result starts as original
    QImage result  = base;

    const int w = result.width();
    const int h = result.height();

    // Build effective mask: union of cumulative mask and new mask
    QImage effectiveMask = mask;
    if (!m_cumulativeBlurMask.isNull()) {
        for (int y = 0; y < h; ++y) {
            QRgb *maskLine     = reinterpret_cast<QRgb *>(effectiveMask.scanLine(y));
            const QRgb *cumMaskLine = reinterpret_cast<const QRgb *>(m_cumulativeBlurMask.constScanLine(y));
            for (int x = 0; x < w; ++x) {
                int grayEff = qGray(maskLine[x]);
                int grayCum = qGray(cumMaskLine[x]);
                if (grayCum > 0 && grayEff == 0) {
                    maskLine[x] = cumMaskLine[x];
                }
            }
        }
    }

    // Blend blurred into result where mask is white
    for (int y = 0; y < h; ++y) {
        QRgb *resLine        = reinterpret_cast<QRgb *>(result.scanLine(y));
        const QRgb *blurLine = reinterpret_cast<const QRgb *>(blurred.constScanLine(y));
        const QRgb *maskLine = reinterpret_cast<const QRgb *>(effectiveMask.constScanLine(y));

        for (int x = 0; x < w; ++x) {
            QRgb m = maskLine[x];
            if (qAlpha(m) > 0 && qGray(m) > 0)
                resLine[x] = blurLine[x];
        }
    }

    m_blurred = QPixmap::fromImage(result);
    
    // Invalidate cache since we modified the image with a mask
    m_cachedBlurStrength = -1;
    
    // Update cumulative blur mask
    if (m_cumulativeBlurMask.isNull()) {
        m_cumulativeBlurMask = mask;
    } else {
        // Already merged above, update it
        m_cumulativeBlurMask = effectiveMask;
    }
}


void SessionController::adoptComputedFullBlur(const QPixmap &pixmap, int strength)
{
    if (m_original.isNull())
        return;

    m_blurred = pixmap;
    m_cachedBlurStrength = strength;
    m_cachedBlurredImage = pixmap;

    // If strength <= 0, clear mask, else mark full image as blurred
    // Do NOT overwrite m_cumulativeBlurMask here. keep mask changes tied to explicit selection edits.
    if (strength <= 0) {
        // when strength == 0 we can clear any full-image cache, but do not touch cumulative mask
    }
}

void SessionController::removeBlur(const QImage &mask)
{
    if (m_original.isNull() || mask.isNull() || mask.size() != m_original.size())
        return;

    // Start from original
    QImage result = m_original.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    const int w = result.width();
    const int h = result.height();

    // Copy blurred pixels EXCEPT where mask is white (those stay original/unblurred)
    // First, rebuild the result with all currently-blurred areas
    if (!m_cumulativeBlurMask.isNull()) {
        // Get the blurred version of the entire image
        int radius = static_cast<int>(std::sqrt(50) * 3.0);  // use midpoint blur for reconstruction
        if (radius > 30) radius = 30;
        
        QImage blurredFull = boxBlur(result, radius);
        
        // Copy blurred where cumulative mask says we should blur, but NOT where removal mask says
        for (int y = 0; y < h; ++y) {
            QRgb *resLine        = reinterpret_cast<QRgb *>(result.scanLine(y));
            const QRgb *blurLine = reinterpret_cast<const QRgb *>(blurredFull.constScanLine(y));
            const QRgb *cumMaskLine = reinterpret_cast<const QRgb *>(m_cumulativeBlurMask.constScanLine(y));
            const QRgb *removalMaskLine = reinterpret_cast<const QRgb *>(mask.constScanLine(y));

            for (int x = 0; x < w; ++x) {
                int grayCum = qGray(cumMaskLine[x]);
                int grayRem = qGray(removalMaskLine[x]);
                
                // Blur if in cumulative mask AND NOT in removal mask
                if (grayCum > 0 && grayRem == 0) {
                    resLine[x] = blurLine[x];
                }
            }
        }
    }

    m_blurred = QPixmap::fromImage(result);
    
    // Invalidate cache since we modified the image
    m_cachedBlurStrength = -1;

    // Update cumulative mask: remove the masked area
    if (!m_cumulativeBlurMask.isNull()) {
        for (int y = 0; y < h; ++y) {
            QRgb *cumMaskLine = reinterpret_cast<QRgb *>(m_cumulativeBlurMask.scanLine(y));
            const QRgb *removalMaskLine = reinterpret_cast<const QRgb *>(mask.constScanLine(y));

            for (int x = 0; x < w; ++x) {
                int grayRem = qGray(removalMaskLine[x]);
                if (grayRem > 0) {
                    cumMaskLine[x] = qRgba(0, 0, 0, 0);  // set to transparent/black
                }
            }
        }
    }
}


void SessionController::pushState()
{
    if (!m_blurred.isNull()) {
        m_undoStack.push_back(m_blurred);
    }
    m_redoStack.clear(); 
}

void SessionController::undo()
{
    if (m_undoStack.isEmpty())
        return;

    // Save current state to redo stack
    m_redoStack.push_back(m_blurred);

    // Pop last state from undo stack
    m_blurred = m_undoStack.last();
    m_undoStack.removeLast();
    
    // Invalidate blur cache since state changed
    m_cachedBlurStrength = -1;
}

void SessionController::redo()
{
    if (m_redoStack.isEmpty())
        return;

    // Push current state to undo stack
    m_undoStack.push_back(m_blurred);

    // Restore next redo state
    m_blurred = m_redoStack.last();
    m_redoStack.removeLast();
    
    // Invalidate blur cache since state changed
    m_cachedBlurStrength = -1;


}

bool SessionController::autoBlurWithPythonDetections(int strength, QString *errorMessage)
{
    if (m_original.isNull()) {
        if (errorMessage) *errorMessage = "No image loaded in session.";
        return false;
    }

    QDir appDir(QCoreApplication::applicationDirPath()); // .../build/bin/Release

    QDir rootDir = appDir;
    rootDir.cdUp(); // Release -> bin
    rootDir.cdUp(); // bin -> build
    rootDir.cdUp(); // build -> Project   (repo root)

    // Now build paths from the root
    const QString scriptPath = rootDir.filePath("src/python/liquor_detect.py");
    const QString modelPath  = rootDir.filePath("models/alcohol-detector.pt"); // adjust name if needed
    qDebug() << "AppDir:" << appDir.absolutePath();
    qDebug() << "Script path:" << scriptPath;
    qDebug() << "Model path:"  << modelPath;

    if (!QFileInfo::exists(scriptPath)) {
        if (errorMessage) {
            *errorMessage = QString("Detection script not found at %1").arg(scriptPath);
        }
        return false;
    }

    if (!QFileInfo::exists(modelPath)) {
        if (errorMessage) {
            *errorMessage = QString("Model .pt not found at %1").arg(modelPath);
        }
        return false;
    }

    // Save the current original image to a temporary PNG for Python
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        if (errorMessage) *errorMessage = "Failed to create temporary directory for detector.";
        return false;
    }

    const QString imagePath = tempDir.path() + "/input.png";
    QImage src = m_original.toImage().convertToFormat(QImage::Format_ARGB32);
    if (!src.save(imagePath, "PNG")) {
        if (errorMessage) *errorMessage = "Failed to write temporary PNG for detector.";
        return false;
    }

    // Build and run the Python process
    QString pythonExe = "python"; // or "python.exe" on Windows; assumes on PATH

    QStringList args;
    args << scriptPath
         << "--image" << imagePath
         << "--model" << modelPath;

    QProcess proc;
    proc.start(pythonExe, args);

    if (!proc.waitForStarted(5000)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to start Python (%1): %2")
                                .arg(pythonExe, proc.errorString());
        }
        return false;
    }

    if (!proc.waitForFinished(-1)) {
        if (errorMessage) *errorMessage = "Python detector did not finish.";
        return false;
    }

    const int exitCode = proc.exitCode();
    QByteArray rawStdOut = proc.readAllStandardOutput();
    QByteArray rawStdErr = proc.readAllStandardError();

    if (exitCode != 0) {
        QString msg = QString::fromLocal8Bit(rawStdErr.isEmpty() ? rawStdOut : rawStdErr);
        if (errorMessage) {
            *errorMessage = QString("Python detector failed (exit %1): %2")
                                .arg(exitCode)
                                .arg(msg.trimmed());
        }
        return false;
    }

    // ---- JSON parsing: use only the last non-empty line from stdout ----
    QByteArray trimmed = rawStdOut.trimmed();
    int lastNewline = trimmed.lastIndexOf('\n');
    QByteArray jsonBytes = (lastNewline == -1)
        ? trimmed
        : trimmed.mid(lastNewline + 1).trimmed();

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Failed to parse detector JSON: %1\nRaw stdout:\n%2")
                                .arg(parseErr.errorString(),
                                     QString::fromLocal8Bit(trimmed));
        }
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray dets = root.value("detections").toArray();
    if (dets.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Detector returned no boxes. Nothing to blur.";
        }
        return false;
    }

    // Build a selection mask the same size as the original image
    const QSize imgSize = m_original.size();
    QImage mask(imgSize, QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);

    QPainter painter(&mask);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    const QRect imgRect(QPoint(0, 0), imgSize);

    for (const QJsonValue &v : dets) {
        QJsonObject o = v.toObject();
        int x = o.value("x").toInt();
        int y = o.value("y").toInt();
        int w = o.value("w").toInt();
        int h = o.value("h").toInt();

        QRect r(x, y, w, h);
        r = r.intersected(imgRect);
        if (!r.isEmpty()) {
            painter.drawRect(r);
        }
    }

    painter.end();

    // Save current blurred state for undo
    pushState();

    // Use your existing selection-based blur pipeline
    applyFakeBlur(strength, mask);

    return true;
}




