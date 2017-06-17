#include <QApplication>
#include <QTextCodec>
#include "ui/MainWindow.h"
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"
#include "tests/TestVersion.h"
#include "AccurateTimer.h"
#include "Version.h"

int main(int argc, char *argv[])
{


    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();

#ifdef QT_DEBUG
    TestCrypt::runTests();
    TestFilesEncrypt::runTests();
    TestVersion::runTests();
#endif

    QApplication a(argc, argv);


    a.setApplicationName("FilesEncrypt");
    a.setApplicationVersion(APP_VERSION);
    a.setOrganizationName("FilesEncrypt");
    a.setOrganizationDomain("filesencrypt.com");

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QSettings::setDefaultFormat(QSettings::IniFormat);

    MainWindow w;

    return a.exec();
}
