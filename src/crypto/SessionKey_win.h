#ifndef SESSIONKEY_WIN_H
#define SESSIONKEY_WIN_H

#include "SessionKeyBase.h"
#include "defines.h"
#include <QObject>

class SessionKey : public SessionKeyBase
{

		Q_OBJECT

	public:
		SessionKey(MainWindow* mainWindow, QString sessionKeyName = QString{".session.key.filesencrypt"});
		QByteArray readSessionKey() override;
	private:
		void encryptAndStoreSessionKey(const char *key) override;
};

#endif // SESSIONKEY_WIN_H
