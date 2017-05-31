#include <QApplication>
#include "ui/MainWindow.h"
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

    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();

    MainWindow w;

    //EVP_cleanup();

    return a.exec();
}
