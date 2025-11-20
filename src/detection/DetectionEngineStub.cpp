// Minimal stub of DetectionEngine to allow building without ONNX Runtime.
// It preserves the public API but performs no real detection.

#include "DetectionEngine.h"
#include <QImage>
#include <QDebug>

DetectionEngine::DetectionEngine() = default;
DetectionEngine::~DetectionEngine() = default;

bool DetectionEngine::loadModel(const QString& modelPath)
{
    Q_UNUSED(modelPath);
    qDebug() << "[DetectionEngineStub] ONNX Runtime not available; using stub.";
    return true; // Pretend model loaded to keep flow working
}

QImage DetectionEngine::runDetection(const QImage& input, std::vector<DetectionResult>& results)
{
    results.clear();
    qDebug() << "[DetectionEngineStub] runDetection called; returning input unchanged.";
    return input; // No overlays or detections
}

