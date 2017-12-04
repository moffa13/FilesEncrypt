#include <QApplication>
#include <QTextCodec>
#include "ui/MainWindow.h"
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"
#include "tests/TestVersion.h"
#include "tests/TestSecureMemBlock.h"
#include "Version.h"
#include <QMessageBox>
#include <QLibraryInfo>
#include <QTranslator>
#include <Logger.h>

int main(int argc, char *argv[])
{

	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();

#ifdef QT_DEBUG
	Logging::Logger::setLogLevel(Logging::DEBUG);
	int result;
	result = TestCrypt::runTests();
	result |= TestFilesEncrypt::runTests();
	result |= TestVersion::runTests();
	result |= TestSecureMemBlock::runTests();
	if(result != 0){
		Logging::Logger::error("Unit test has failed. Please fix errors");
		return result;
	}
#ifdef UNIT_TEST
	EVP_cleanup();
	return 0;
#endif

#else // QT_DEBUG
	Logging::Logger::setLogLevel(Logging::ERROR);
#endif

#ifndef UNIT_TEST
	QApplication a{argc, argv};

	QTranslator translator;
	translator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	QTranslator translator2;
	translator2.load(QLocale::system().name(), ":/lang");

	qApp->installTranslator(&translator);
	qApp->installTranslator(&translator2);

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
		QMessageBox::information(&w, MainWindow::tr("Mise à jour"), MainWindow::tr("La mise à jour a correctement été installée."), QMessageBox::Ok);
	}

	return a.exec();
#endif
}
