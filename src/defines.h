#ifndef DEFINES_H
#define DEFINES_H

#include <QTableWidgetItem>
#include <QFutureWatcher>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>
#include "defines.h"

#define QPAIR_CRYPT_DEF QPair<QString, EncryptDecrypt_s>

typedef struct CertInfos CertInfos;
struct CertInfos{
    std::string country_code;
    std::string organisation_name;
    std::string website_url;
    std::string password;
};

enum AESSIZE{
    S128 = 16,
    S192 = 24,
    S256 = 32
};

enum EncryptDecrypt{
    NOT_FINISHED = - 1,
    ENCRYPT = 0,
    DECRYPT = 1,
    PARTIAL = 2
};

typedef struct {
    EncryptDecrypt *state;
    quint8 offsetBeforeContent;
} EncryptDecrypt_light;

typedef struct {
    EncryptDecrypt state;
    quint32 version;
    QByteArray iv;
    quint8 offsetBeforeContent;
    bool filenameChanged;
    QByteArray newFilename;
} EncryptDecrypt_s;

typedef struct{
    QStringList files;
    quint64 size;
} FilesAndSize;

// Entries infos
typedef struct CryptInfos CryptInfos;
struct CryptInfos{
    QMap<QString, EncryptDecrypt_light> files;
    bool isFile;
    QString type;
    QString name;
    QString size;
    EncryptDecrypt* state;
    QString stateStr;
    QFutureWatcher<QPAIR_CRYPT_DEF>* watcher;
    QFutureWatcher<FilesAndSize>* recursiveWatcher;
};

typedef struct {
    QString name;
    quint8 offsetBeforeContent;
    unsigned size;
    EncryptDecrypt state;
    bool success;
} finfo_s;

#endif // DEFINES_H
