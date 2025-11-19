#include <QApplication>

#include "MainWindow.h"
#include "SessionController.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    SessionController controller;

    // GUI -> core
    QObject::connect(&window, &MainWindow::importImageRequested,
                     &controller, &SessionController::handleImportImage);
    QObject::connect(&window, &MainWindow::runDetectionRequested,
                     &controller, &SessionController::handleRunDetection);
    QObject::connect(&window, &MainWindow::exportImageRequested,
                     &controller, &SessionController::handleExportImage);

    // core -> GUI
    QObject::connect(&controller, &SessionController::imageUpdated,
                     &window, &MainWindow::onImageUpdated);
    QObject::connect(&controller, &SessionController::statusMessage,
                     &window, &MainWindow::showStatusMessage);

    window.show();
    return app.exec();
}
