#include <QApplication>
#include <QTextCodec>
#include "ui/MainWindow.h"
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"
#include "tests/TestVersion.h"
#include "Version.h"
#include <QMessageBox>
#include <Logger.h>

int main(int argc, char *argv[])
{

#ifndef UNIT_TEST
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
#endif

#ifdef QT_DEBUG
    Logging::Logger::setLogLevel(Logging::DEBUG);
    int result;
    result = TestCrypt::runTests();
    result |= TestFilesEncrypt::runTests();
    result |= TestVersion::runTests();
    if(result != 0){
        Logging::Logger::error("Unit test has failed. Please fix errors");
        return result;
    }
#ifdef UNIT_TEST
    return 0;
#endif

#else // QT_DEBUG
    Logging::Logger::setLogLevel(Logging::ERROR);
#endif

#ifndef UNIT_TEST
    QApplication a{argc, argv};

    a.setApplicationName("FilesEncrypt");
    a.setApplicationVersion(APP_VERSION);
    a.setOrganizationName("FilesEncrypt");
    a.setOrganizationDomain("filesencrypt.com");

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QSettings::setDefaultFormat(QSettings::IniFormat);

    MainWindow w;
    if(argc > 1 && strcmp(argv[1], "update_done") == 0){
        // Delete old
        QFile::remove(qApp->applicationFilePath() + ".old");
        QMessageBox::information(&w, "Mise à jour", "La mise à jour a correctement été installée.", QMessageBox::Ok);
    }

    return a.exec();
#endif
}
