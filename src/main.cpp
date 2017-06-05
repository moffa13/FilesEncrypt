#include <QApplication>
#include "ui/MainWindow.h"
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"

int main(int argc, char *argv[])
{

#ifdef QT_DEBUG
    TestCrypt::runTests();
    TestFilesEncrypt::runTests();
#endif

    QApplication a(argc, argv);
    a.setApplicationName("FilesEncrypt");
    a.setOrganizationName("FilesEncrypt");
    a.setOrganizationDomain("filesencrypt.com");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();

    MainWindow w;

    return a.exec();
}
