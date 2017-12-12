#include "SessionKey.h"
#include <QApplication>
#include "FilesEncrypt.h"
#include <QMessageBox>
#include "ui/MainWindow.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

SessionKey::SessionKey(MainWindow* mainWindow, QString sessionKeyName) : _sessionKeyName(std::move(sessionKeyName)), _mainWindow(mainWindow){
	QObject::connect(_mainWindow->m_choose_key, SIGNAL(userExit()), this, SIGNAL(finishedAction()));
}

/**
 * Encrypts/Decrypts the specified file using the _mainWindow and emits finishedAction when the file have been processed
 * @brief SessionKey::action
 * @param item Concerned files
 * @param action
 */
void SessionKey::action(QString const& item, EncryptDecrypt action){
	QStringList items;
	items << item;
	this->action(items, action);
}

/**
 * Encrypts/Decrypts the specified files using the _mainWindow and emits finishedAction when all files have been processed
 * @brief SessionKey::action
 * @param item Concerned files
 * @param action
 */
void SessionKey::action(QStringList const& items, EncryptDecrypt action){
	if(items.contains(QDir::toNativeSeparators(QApplication::applicationDirPath() + "/" + _sessionKeyName))){
		QMessageBox::critical(nullptr, tr("Clé de session"), tr("Vous ne pouvez pas modifier la clé de session"), QMessageBox::Ok);
	}else{
		_mainWindow->addWhateverToList(items);
		_mainWindow->action(action);
	}
	emitIfNoMoreEncrypt();
}

/**
 * Reads the user session encrypted file, decrypts it & return 32 bytes aes key
 * If the session key file does not exist, returns empty QByteArray
 * @brief SessionKey::readSessionKey
 * @return
 */
QByteArray SessionKey::readSessionKey(){
	QFile sessionKeyFile{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	if(!sessionKeyFile.exists() || !sessionKeyFile.open(QFile::ReadWrite)){
		return QByteArray{};
	}
	QByteArray content = sessionKeyFile.readAll();
	sessionKeyFile.close();

	DATA_BLOB crypted;
	crypted.cbData = content.length();
	crypted.pbData = (BYTE*)content.constData();

	DATA_BLOB uncrypted;
	CryptUnprotectData(&crypted, NULL, NULL, NULL, NULL, 0, &uncrypted);
	QByteArray ret{reinterpret_cast<const char*>(uncrypted.pbData), static_cast<int>(uncrypted.cbData)};
	LocalFree(uncrypted.pbData);

	return ret;
}

void SessionKey::emitIfNoMoreEncrypt(){
	if(FilesEncrypt::getPendingCrypt() == 0){
		emit finishedAction();
	}
}

/**
 * @brief SessionKey::encryptAndStoreSessionKey
 * @param key Uncrypted 32 bytes aes key
 */
void SessionKey::encryptAndStoreSessionKey(const char* key){
	DATA_BLOB aes;
	aes.cbData = 32;
	aes.pbData = (BYTE*)key;

	DATA_BLOB sessionKey;

	CryptProtectData(&aes, NULL, NULL, NULL, NULL, 0, &sessionKey);

	QFile sessionKeyFile{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	sessionKeyFile.open(QFile::ReadWrite);
	sessionKeyFile.write(reinterpret_cast<const char*>(sessionKey.pbData), sessionKey.cbData);
	LocalFree(sessionKey.pbData);
	sessionKeyFile.close();
}

void SessionKey::checkForSessionKey(bool warn){
	QFile sessionKey{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	bool keyReadyB = false; // Delay the emit a the end
	if(!sessionKey.exists()){
		if(warn)
			QMessageBox::information(nullptr, tr("Créer une clé de session"), tr("Vous ne disposez pas encore de clé de session, merci de sélectionner une clé ou d'en créer une"), QMessageBox::Ok);
		if(_mainWindow->beSureKeyIsSelectedAndValid([this, warn](){checkForSessionKey(warn);}, false)){ // Asks user to select/create aes
			SecureMemBlock aes = _mainWindow->m_filesEncrypt->getAES();
			encryptAndStoreSessionKey(reinterpret_cast<const char*>(aes.getData()));
			keyReadyB = true;
		}
	}else{
		QByteArray sessionKey = readSessionKey();
		_mainWindow->m_filesEncrypt = new FilesEncrypt(sessionKey.constData());
		_mainWindow->keySelected(); // Tell the window a key has been added
		keyReadyB = true;
	}

	if(_mainWindow->m_filesEncrypt){
		QObject::connect(_mainWindow->m_filesEncrypt, &FilesEncrypt::file_done, [this](){
			emitIfNoMoreEncrypt();
		});
	}

	if(keyReadyB) emit keyReady();
}
