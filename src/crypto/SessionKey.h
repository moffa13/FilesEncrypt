#ifndef SESSIONKEY_H
#define SESSIONKEY_H

#include "SecureMemBlock.h"
#include "defines.h"
#include <QObject>

class MainWindow;

class SessionKey : public QObject
{

		Q_OBJECT

	public:
		SessionKey(MainWindow* mainWindow, QString sessionKeyName = QString{".session.key.filesencrypt"});
		void action(const QString &item, EncryptDecrypt action);
		void action(const QStringList &items, EncryptDecrypt action);
		QByteArray readSessionKey();
		void checkForSessionKey(bool warn = true);
	private:
		const QString _sessionKeyName;
		MainWindow* _mainWindow;
		void encryptAndStoreSessionKey(const char *key);
		SecureMemBlock *_secureAes;
		void emitIfNoMoreEncrypt();
	Q_SIGNALS:
		void finishedAction();
		void keyReady();
};

#endif // SESSIONKEY_H
