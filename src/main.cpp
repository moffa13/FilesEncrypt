#include <QApplication>
#include <QTextCodec>
#include "ui/MainWindow.h"
#include "Version.h"
#include <QMessageBox>
#include <QLibraryInfo>
#include <QTranslator>
#include <Logger.h>

#ifdef Q_OS_WIN
#include "SessionKey.h"
#endif

#ifdef QT_DEBUG
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"
#include "tests/TestVersion.h"
#include "tests/TestSecureMemBlock.h"
#endif

int main(int argc, char *argv[])
{

	OpenSSL_add_all_algorithms();
	ERR_load_BIO_strings();

#ifdef QT_DEBUG
	Logging::Logger::setLogLevel(Logging::DEBUG);
	int result;
	result = TestSecureMemBlock::runTests();
	result |= TestCrypt::runTests();
	result |= TestFilesEncrypt::runTests();
	result |= TestVersion::runTests();
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

	EncryptDecrypt action = NOT_FINISHED;

	if(argc > 1 && strcmp(argv[1], "update_done") == 0){
		// Delete old
		QFile::remove(qApp->applicationFilePath() + ".old");
		QMessageBox::information(&w, MainWindow::tr("Mise à jour"), MainWindow::tr("La mise à jour a correctement été installée."), QMessageBox::Ok);
	}
#ifdef Q_OS_WIN
	else if(argc > 1 && strcmp(argv[1], "encrypt") == 0){
		action = ENCRYPT;
	}else if(argc > 1 && strcmp(argv[1], "decrypt") == 0){
		action = DECRYPT;
	}

	// Encryption from contextual menu needed
	if(action != NOT_FINISHED){
		SessionKey s{&w};
		QEventLoop loop;
		QObject::connect(&s, &SessionKey::finishedAction, [&loop](){
			loop.exit();
		});
		QObject::connect(&s, &SessionKey::keyReady, [&s, argv, action](){
			s.action(argv[2],  action);
		});
		QTimer::singleShot(0, [&s](){ // To be sure loop is executing
			s.checkForSessionKey();
		});
		loop.exec();
		QTimer::singleShot(0, [](){ // Tell the main event loop to exit
			qApp->exit();
		});
	}else{
		w.show();
	}
#else
	w.show();
#endif // Q_OS_WIN
	return a.exec();
#endif
}
