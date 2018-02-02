#include <QApplication>
#include <QTextCodec>
#include "ui/MainWindow.h"
#include "Version.h"
#include <QMessageBox>
#include <QLibraryInfo>
#include <QTranslator>
#include <Logger.h>
#include "Init.h"

#ifdef Q_OS_WIN
#include "crypto/SessionKey.h"
#include <Windows.h>
#endif

#ifdef QT_DEBUG
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"
#include "tests/TestVersion.h"
#include "tests/TestSecureMemBlock.h"
#endif

int main(int argc, char *argv[]){

	// Init openssl & libgcrypt
	Init::init();

	QApplication a{argc, argv};

// If in debug mode, run unit tests before starting program
#ifdef QT_DEBUG
	Logging::Logger::setLogLevel(Logging::DEBUG);
	int result;
	result = TestSecureMemBlock::runTests();
	result |= TestCrypt::runTests();
	result |= TestFilesEncrypt::runTests();
	result |= TestVersion::runTests();
	if(result != 0){
		Init::deInit();
		Logging::Logger::error("Unit test has failed. Please fix errors");
		return result;
	}
#ifdef UNIT_TEST
	Init::deInit();
	return 0;
#endif

#else // !QT_DEBUG
	Logging::Logger::setLogLevel(ERROR);
#endif // QT_DEBUG

#ifndef UNIT_TEST

	ushort** argvw = nullptr;
	int argcw;

#ifdef Q_OS_WIN
	argvw = (ushort**)CommandLineToArgvW(GetCommandLineW(), &argcw);
#else
	argvw = argv;
	argcw = argc;
#endif

	if(argc > 2 && strcmp(argv[1], "getState") == 0){ // This has to be fast for explorer

		Init::deInit();

		unsigned uncrypted = 0;
		unsigned crypted = 0;

		for(int i = 2; i < argcw; ++i){

			QString fileStr = QString::fromUtf16(argvw[i]);

			QFileInfo fInfo{fileStr};
			if(!fInfo.exists()) continue;

			if(fInfo.isDir()){
				EncryptDecrypt infos = FilesEncrypt::guessEncrypted(QDir{fileStr});
				if(infos == PARTIAL){ // If one dir is partially encrypted, return PARTIAL
					return 3;
				}else if(infos == ENCRYPT){
					crypted++;
				}else{
					uncrypted++;
				}
			}else{
				QFile f{fileStr};
				if(!f.open(QFile::ReadWrite)) continue;
				EncryptDecrypt_s infos = FilesEncrypt::guessEncrypted(f);
				f.close();
				if(infos.state == ENCRYPT){
					crypted++;
				}else{
					uncrypted++;
				}

			}

			if(crypted != 0 && uncrypted != 0){
				return 3;
			}
		}

		if(crypted == 0){
			return 1; // Decrypted
		}else if(uncrypted == 0){
			return 2;
		}
	}

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
	w.show();

	EncryptDecrypt action = NOT_FINISHED;

	if(argc > 1 && strcmp(argv[1], "update_done") == 0){
		// Delete old
		QFile::remove(qApp->applicationFilePath() + ".old");
		QMessageBox::information(&w, MainWindow::tr("Mise à jour"), MainWindow::tr("La mise à jour a correctement été installée."), QMessageBox::Ok);
	}
#ifdef Q_OS_WIN
	else if(argc > 2 && strcmp(argv[1], "encrypt") == 0){
		action = ENCRYPT;
	}else if(argc > 2 && strcmp(argv[1], "decrypt") == 0){
		action = DECRYPT;
	}

	// Encryption from contextual menu needed
	if(action != NOT_FINISHED){
		SessionKey s{&w};
		QEventLoop loop;
		QObject::connect(&s, &SessionKey::finishedAction, [&loop](){
			loop.exit();
		});
		QObject::connect(&s, &SessionKey::keyReady, [&s, argcw, argvw, action](){
			if(argcw > 3){
				QStringList files;
				for(int i = 2; i < argcw; ++i){
					files << QString::fromUtf16(argvw[i]);
				}
				s.action(files, action);
			}else{
				s.action(QString::fromUtf16(argvw[2]),  action);
			}
		});
		QTimer::singleShot(0, [&s](){ // To be sure loop is executing
			s.checkForSessionKey();
		});
		loop.exec();
		QTimer::singleShot(0, [](){ // Tell the main event loop to exit
			qApp->exit();
		});
	}
#endif // Q_OS_WIN
	auto ret = a.exec();
	Init::deInit();
	LocalFree(argvw);
	return ret;
#endif // UNIT_TEST
}
