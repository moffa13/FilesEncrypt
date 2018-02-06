
#ifndef SESSIONKEY_LINUX_H
#define SESSIONKEY_LINUX_H

#include <QByteArray>

#include "SessionKeyBase.h"
#include "ui/MainWindow.h"

class SessionKey : public SessionKeyBase
{

public:
    SessionKey(MainWindow* mainWindow, QString sessionKeyName = QString{".session.key.filesencrypt"});
    QByteArray readSessionKey() override;
private:
    void encryptAndStoreSessionKey(const char *key) override;

};

#endif // SESSIONKEY_LINUX_H

