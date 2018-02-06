#include "SessionKey_linux.h"
#include <QApplication>
#include <gnome-keyring-1/gnome-keyring.h>
#include "utilities.h"
#include <cstdio>

SessionKey::SessionKey(MainWindow *mainWindow, QString sessionKeyName) : SessionKeyBase(mainWindow, sessionKeyName) {}

QByteArray SessionKey::readSessionKey(){
    char* password;
    gnome_keyring_find_password_sync(GNOME_KEYRING_NETWORK_PASSWORD,
                                     &password,
                                     "user", "me",
                                     NULL
                                     );

    FilesEncrypt f((QApplication::applicationDirPath() + "/" + _sessionKeyName).toStdString());
    f.requestAesDecrypt(std::string(password), nullptr);
    gnome_keyring_free_password(password);
    return QByteArray(reinterpret_cast<const char*>(f.getAES().getData()), 32);
}

void SessionKey::encryptAndStoreSessionKey(const char *key){

    QString random = utilities::randomString(16);

    gnome_keyring_store_password_sync(GNOME_KEYRING_NETWORK_PASSWORD,
                                      GNOME_KEYRING_DEFAULT,
                                      "FilesEncrypt Secure Key",
                                      random.toStdString().c_str(),
                                      "user", "me",
                                      NULL
                                      );

    FilesEncrypt::genKey(QApplication::applicationDirPath() + "/" + _sessionKeyName, random, reinterpret_cast<const unsigned char*>(key));
}
