#ifndef FILESENCRYPT_H
#define FILESENCRYPT_H

#include <QFile>
#include "Crypt.h"
#include <QDir>
#include <QStringList>
#include <QMutex>

enum EncryptDecrypt{
    NOT_FINISHED = - 1,
    ENCRYPT = 0,
    DECRYPT = 1,
    PARTIAL = 2
};

typedef struct{
    QStringList files;
    quint64 size;
} FilesAndSize;

class FilesEncrypt : public QObject
{

    Q_OBJECT

public:
    FilesEncrypt(std::string const &key_file);
    FilesEncrypt(FilesEncrypt const&) = delete;
    FilesEncrypt& operator =(FilesEncrypt const&) = delete;
    bool encryptFile(QFile* file, EncryptDecrypt op);
    static bool genKey(QString const& file, QString const& password);
    bool init();
    static EncryptDecrypt guessEncrypted(QFile& f);
    static EncryptDecrypt guessEncrypted(QDir& dir);
    static EncryptDecrypt guessEncrypted(QByteArray const& content);
    static FilesAndSize getFilesFromDirRecursive(QDir const& dir);
    bool requestAesDecrypt(std::string const& password, bool* passOk = NULL);
    bool isAesUncrypted();
    const unsigned char* getAES() const;
    static unsigned getPendingCrypt();

private:
    std::string m_key_file;
    unsigned char* m_aes_crypted;
    size_t m_aes_crypted_length;
    unsigned char* m_aes_decrypted = NULL;
    std::string m_private_key_crypted;
    static unsigned m_pendingCrypt;
    static QMutex m_mutex;
    void addPendingCrypt();
    void removePendingCrypt();
    void startDeleteAesTimer();
Q_SIGNALS:
    void encrypt_updated(qint32 progress);
    void decrypt_updated(qint32 progress);
    void file_done();
};

#endif // FILESENCRYPT_H
