#include "ContextualMenuToggler.h"
#include <QSettings>
#include <QApplication>
#include <QDir>

void ContextualMenuToggler::toggleCryptUncryptOptions(bool enable){
	if(enable) setCryptUncryptOptions();
	else unsetCryptUncryptOptions();
}

void ContextualMenuToggler::setCryptUncryptOptions(){
#ifdef Q_OS_WIN
	QSettings settings{"HKEY_CURRENT_USER\\Software\\Classes\\*\\shell", QSettings::NativeFormat};
	settings.beginGroup("FilesEncrypt_encrypt");
	settings.setValue("Default", tr(qPrintable("Crypter avec " + QApplication::applicationName())));
	settings.beginGroup("command");
	settings.setValue("Default", QDir::toNativeSeparators(QApplication::applicationFilePath() + " encrypt %1"));
	settings.endGroup();
	settings.endGroup();
	settings.beginGroup("FilesEncrypt_decrypt");
	settings.setValue("Default", tr(qPrintable("Décrypter avec " + QApplication::applicationName())));
	settings.beginGroup("command");
	settings.setValue("Default", QDir::toNativeSeparators(QApplication::applicationFilePath() + " decrypt %1"));
	settings.endGroup();
	settings.endGroup();
#endif;
}

void ContextualMenuToggler::unsetCryptUncryptOptions(){
#ifdef Q_OS_WIN
	QSettings settings{"HKEY_CURRENT_USER\\Software\\Classes\\*\\shell", QSettings::NativeFormat};
	settings.remove("FilesEncrypt_encrypt");
	settings.remove("FilesEncrypt_decrypt");
#endif;
}


