#include <QApplication>
#include "MainWindow.h"
#include <QMutexLocker>
#include <QMap>
#include "tests/TestCrypt.h"

int main(int argc, char *argv[])
{

#ifdef QT_DEBUG
    TestCrypt::runTests();
#endif

    QApplication a(argc, argv);
    a.setApplicationName("FilesEncrypt");
    a.setOrganizationName("Bigcoding");
    a.setOrganizationDomain("Bigcoding.com");

    qDebug() << QDir::tempPath();

    MainWindow w;

    return a.exec();
}
