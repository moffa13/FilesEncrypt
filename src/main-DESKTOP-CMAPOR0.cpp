#include <QApplication>
#include "MainWindow.h"
#include <QMutexLocker>
#include <QMap>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    a.setApplicationName("FilesEncrypt");
    a.setOrganizationName("Bigcoding");
    a.setOrganizationDomain("Bigcoding.com");

    qDebug() << QDir::tempPath();

    MainWindow w;

    return a.exec();
}
