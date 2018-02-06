#ifndef SESSIONKEY_H
#define SESSIONKEY_H

#include "SecureMemBlock.h"
#include "SessionKeyBase.h"
#include "defines.h"
#include <QObject>

class MainWindow;

class SessionKey : public SessionKeyBase
{

		Q_OBJECT

	public:
		SessionKey(MainWindow* mainWindow, QString sessionKeyName = QString{".session.key.filesencrypt"});
        QByteArray readSessionKey() override;
	private:
        void encryptAndStoreSessionKey(const char *key) override;
	Q_SIGNALS:
		void finishedAction();
		void keyReady();
};

#endif // SESSIONKEY_H
