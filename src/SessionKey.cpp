#include "SessionKey.h"
#include <QApplication>
#include "FilesEncrypt.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

SessionKey::SessionKey(MainWindow* mainWindow, QString sessionKeyName) : _sessionKeyName(std::move(sessionKeyName)), _mainWindow(mainWindow){}

void SessionKey::action(QString const& item, EncryptDecrypt action){
	_mainWindow->addWhateverToList(item);
	QObject::connect(_mainWindow->m_filesEncrypt, &FilesEncrypt::file_done, [this](){
		if(FilesEncrypt::getPendingCrypt() == 0){
			emit finishedAction();
		}
	});
	_mainWindow->action(action);
}

void SessionKey::action(QStringList const& items, EncryptDecrypt action){
	_mainWindow->addWhateverToList(items);
	_mainWindow->action(action);
}

QByteArray SessionKey::readSessionKey(){
	QFile sessionKeyFile{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	sessionKeyFile.open(QFile::ReadWrite);
	return sessionKeyFile.readAll();
}

void SessionKey::encryptAndStoreSessionKey(const char* key){
	DATA_BLOB aes;
	aes.cbData = 32;
	aes.pbData = (BYTE*)key;

	DATA_BLOB sessionKey;

	CryptProtectData(&aes, NULL, NULL, NULL, NULL, 0, &sessionKey);

	QFile sessionKeyFile{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	sessionKeyFile.open(QFile::ReadWrite);
	sessionKeyFile.write(reinterpret_cast<const char*>(sessionKey.pbData), sessionKey.cbData);
	sessionKeyFile.close();
}

void SessionKey::checkForSessionKey(){
	QFile sessionKey{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	if(!sessionKey.exists()){
		if(_mainWindow->beSureKeyIsSelectedAndValid([this](){checkForSessionKey();}, false)){
			SecureMemBlock aes = _mainWindow->m_filesEncrypt->getAES();
			encryptAndStoreSessionKey(reinterpret_cast<const char*>(aes.getData()));
			emit keyReady();
		}
	}else{
		QByteArray sessionKey = readSessionKey();
		_mainWindow->m_filesEncrypt = new FilesEncrypt(sessionKey.constData());
		_mainWindow->keySelected(); // Tell the window a key has been added
		emit keyReady();
	}
}
