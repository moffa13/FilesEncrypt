#include <QApplication>
#include "ui/MainWindow.h"
#include <QMessageBox>
#include <QLibraryInfo>
#include <QTranslator>
#include <Logger.h>
#include "Init.h"
#include "crypto/SessionKey.h"
#include "ui/FilesListModel.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifdef QT_DEBUG
#include "tests/TestCrypt.h"
#include "tests/TestFilesEncrypt.h"
#include "tests/TestVersion.h"
#include "tests/TestSecureMemBlock.h"
#include "tests/TestSessionKey.h"
#endif

#ifdef Q_OS_WIN
static ushort** argvw = nullptr;
#else
static char** argvw = nullptr;
#endif
static int argcw;

QString getArgv(int i){
#ifdef Q_OS_WIN
    return QString::fromUtf16(reinterpret_cast<char16_t*>(argvw[i]));
#else
	return QString(argvw[i]);
#endif
}

int main(int argc, char *argv[]){

	// Init openssl & libgcrypt
	Init::init();

	QApplication a{argc, argv};

    a.setApplicationName("FilesEncrypt");
    a.setApplicationVersion(APP_VERSION);
    a.setOrganizationName("FilesEncrypt");
    a.setOrganizationDomain("filesencrypt.com");

    QSettings::setDefaultFormat(QSettings::IniFormat);

// If in debug mode, run unit tests before starting program
#ifdef QT_DEBUG
#ifndef QT_NO_UNIT_TEST
	Logging::Logger::setLogLevel(Logging::DEBUG);
	int result;
	result = TestSecureMemBlock::runTests();
	result |= TestCrypt::runTests();
    result |= TestSessionKey::runTests();
	result |= TestFilesEncrypt::runTests();
	result |= TestVersion::runTests();
	if(result != 0){
		Init::deInit();
		Logging::Logger::error("Unit test has failed. Please fix errors");
		return result;
	}
#endif
#ifdef UNIT_TEST
	Init::deInit();
	return 0;
#endif

#else // !QT_DEBUG
	Logging::Logger::setLogLevel(ERROR);
#endif // QT_DEBUG

#ifndef UNIT_TEST

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

			QString fileStr = getArgv(i);

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
    if(!translator.load("qt_" + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        qDebug() << "Qt own translations not loaded";

    QTranslator translator2;
    if(!translator2.load(QLocale::system().name(), ":/lang"))
        qDebug() << "Translations not loaded";

    qApp->installTranslator(&translator);
    qApp->installTranslator(&translator2);

	MainWindow w;
	w.show();

	EncryptDecrypt action = NOT_FINISHED;

	if(argc > 1 && strcmp(argv[1], "update_done") == 0){
		// Delete old
		QFile::remove(qApp->applicationFilePath() + ".old");
		QMessageBox::information(&w, MainWindow::tr("Mise à jour"), MainWindow::tr("La mise à jour a correctement été installée."), QMessageBox::Ok);
	}else if(argc > 2 && strcmp(argv[1], "encrypt") == 0){
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
		QObject::connect(&s, &SessionKey::keyReady, [&s, action](){
			if(argcw > 3){
				QStringList files;
				for(int i = 2; i < argcw; ++i){
					files << getArgv(i);
				}
				s.action(files, action);
			}else{
				s.action(getArgv(2),  action);
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

#ifdef Q_OS_WIN
	LocalFree(argvw);
#endif // Q_OS_WIN
	auto ret = a.exec();
	Init::deInit();
	return ret;
#endif // UNIT_TEST
}
