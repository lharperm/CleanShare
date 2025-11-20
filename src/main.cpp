#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon appIcon(":/CleanShareLogo.ico"); // path from Logo.qrc
    a.setWindowIcon(appIcon);

    MainWindow w;
    w.show();
    return a.exec();
}
