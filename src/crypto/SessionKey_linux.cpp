#include "SessionKey_linux.h"
#include <QApplication>
#include "utilities.h"

SessionKey::SessionKey(MainWindow *mainWindow, QString sessionKeyName) : SessionKeyBase(mainWindow, sessionKeyName) {

    SecretSchemaAttribute secretSchemaAttribute;
    secretSchemaAttribute.name = "user";
    secretSchemaAttribute.type = SECRET_SCHEMA_ATTRIBUTE_STRING;


    _schema.name = "com.filesencrypt.keys";
    _schema.flags = SECRET_SCHEMA_NONE;
    _schema.attributes[0] = secretSchemaAttribute;

}

QByteArray SessionKey::readSessionKey(){

    GError *error = nullptr;
    char* password = secret_password_lookup_nonpageable_sync(&_schema,
                                NULL,
                                &error,
                                "user", "me",
                                NULL);

    FilesEncrypt f((QApplication::applicationDirPath() + "/" + _sessionKeyName).toStdString());
    f.requestAesDecrypt(std::string(password), nullptr);
    secret_password_free(password);
    return QByteArray(reinterpret_cast<const char*>(f.getAES().getData()), 32);
}

void SessionKey::encryptAndStoreSessionKey(const char *key){

    QString random = utilities::randomString(16);

    GError *error = nullptr;

    secret_password_store_sync(&_schema,
                               SECRET_COLLECTION_DEFAULT,
                               "FilesEncrypt Secure Key",
                               random.toStdString().c_str(),
                               NULL,
                               &error,
                               "user", "me",
                               NULL);

    FilesEncrypt::genKey(QApplication::applicationDirPath() + "/" + _sessionKeyName, random, reinterpret_cast<const unsigned char*>(key));
}
