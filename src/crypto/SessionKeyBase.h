#ifndef SESSIONKEYBASE_H
#define SESSIONKEYBASE_H

#include <QObject>
#include "defines.h"
#include "SecureMemBlock.h"

class MainWindow;

class SessionKeyBase : public QObject
{

		Q_OBJECT

public:
	SessionKeyBase(MainWindow* mainWindow, QString sessionKeyName = QString{".session.key.filesencrypt"});
	void action(const QString &item, EncryptDecrypt action);
	void action(const QStringList &items, EncryptDecrypt action);
	void checkForSessionKey(bool warn = true);
	virtual QByteArray readSessionKey() = 0;
private:
	MainWindow* _mainWindow;
	virtual void encryptAndStoreSessionKey(const char *key) = 0;
	SecureMemBlock *_secureAes;
	void emitIfNoMoreEncrypt();
protected:
		const QString _sessionKeyName;
Q_SIGNALS:
		void finishedAction();
		void keyReady();
};

#endif // SESSIONKEYBASE_H
