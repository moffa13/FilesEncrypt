#include "SessionKey_win.h"

#include <Windows.h>
#include <QApplication>
#include <QFile>

SessionKey::SessionKey(MainWindow *mainWindow, QString sessionKeyName) : SessionKeyBase(mainWindow, sessionKeyName) {}

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

/**
 * Encrypts the raw aes key & stores it
 * @brief SessionKey::encryptAndStoreSessionKey
 * @param key Uncrypted 32 bytes aes key
 */
void SessionKey::encryptAndStoreSessionKey(const char* key){

	BYTE* aesCopy = reinterpret_cast<BYTE*>(malloc(32));
	memcpy(aesCopy, key, 32);

	DATA_BLOB aes;
	aes.cbData = 32;
	aes.pbData = aesCopy;

	DATA_BLOB sessionKey;

	CryptProtectData(&aes, NULL, NULL, NULL, NULL, 0, &sessionKey);

	free(aesCopy);

	QFile sessionKeyFile{QApplication::applicationDirPath() + "/" + _sessionKeyName};
	sessionKeyFile.open(QFile::ReadWrite);
	sessionKeyFile.write(reinterpret_cast<const char*>(sessionKey.pbData), sessionKey.cbData);
	LocalFree(sessionKey.pbData);
	sessionKeyFile.close();
}
