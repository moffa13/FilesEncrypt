#ifndef SESSIONKEY_H
#define SESSIONKEY_H

#include "ui/MainWindow.h"
#include "SecureMemBlock.h"


class SessionKey : public QObject
{

		Q_OBJECT

	public:
		SessionKey(MainWindow* mainWindow, QString sessionKeyName = QString{".session.key.filesencrypt"});
		void action(const QString &item, EncryptDecrypt action);
		void action(const QStringList &items, EncryptDecrypt action);
		void checkForSessionKey();
	private:
		const QString _sessionKeyName;
		MainWindow* _mainWindow;
		void encryptAndStoreSessionKey(const char *key);
		SecureMemBlock *_secureAes;
		QByteArray readSessionKey();
		void emitIfNoMoreEncrypt();
	Q_SIGNALS:
		void finishedAction();
		void keyReady();
};

#endif // SESSIONKEY_H
