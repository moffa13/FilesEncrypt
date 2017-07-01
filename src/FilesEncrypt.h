#ifndef FILESENCRYPT_H
#define FILESENCRYPT_H

#include <QFile>
#include "Crypt.h"
#include <QDir>
#include <QStringList>
#include <QMutex>
#include "openssl/aes.h"
#include <QTimer>

class FilesEncrypt : public QObject
{

    Q_OBJECT

public:
    explicit FilesEncrypt(std::string key_file);
    explicit FilesEncrypt(const char* aes);
    FilesEncrypt(FilesEncrypt const&) = delete;
    FilesEncrypt& operator =(FilesEncrypt const&) = delete;
    ~FilesEncrypt();
    finfo_s encryptFile(QFile* file, EncryptDecrypt op, bool filenameNeedsEncryption) const;
    bool readFromFile();
    static bool genKey(QString const& file, QString const& password, const unsigned char *aes_copy = nullptr);
    static EncryptDecrypt_s guessEncrypted(QFile& f);
    static EncryptDecrypt guessEncrypted(QDir const& dir);
    static EncryptDecrypt_s guessEncrypted(QByteArray const& content);
    static FilesAndSize getFilesFromDirRecursive(QDir const& dir);
    static unsigned getPendingCrypt();
    bool requestAesDecrypt(std::string const& password, bool* passOk = NULL);
    bool isAesDecrypted() const;
    inline bool isFileKeyLoaded() const { return m_key_file_loaded; }
    const unsigned char* getAES() const;
    void setAES(const char* aes);
    void unsetAES();
    static const char compare[];
    static const size_t COMPARE_SIZE;
    static const size_t VERSION_LENGTH; // V00000;
    static const size_t SIZE_BEFORE_CONTENT;
    static const char PRIVATE_ENCRYPT_AES_SEPARATOR;
    static size_t getEncryptedSize(int message_length);
    static QByteArray getEncryptBlob(const char* iv, quint32 version, bool filenameChanged, const char* newFilename, int newFilename_size);
private:
    void init();
    std::string m_key_file;
    unsigned char* m_aes_crypted = nullptr;
    unsigned char* m_aes_decrypted = nullptr;
    bool m_aes_decrypted_set = false;
    size_t m_aes_crypted_length;
    bool m_key_file_loaded = false;
    std::string m_private_key_crypted;
    QTimer m_deleteAESTimer;
    static unsigned s_pendingCrypt;
    static QMutex s_mutex;
    static void addPendingCrypt();
    static void removePendingCrypt();
    void startDeleteAesTimer();
Q_SIGNALS:
    void encrypt_updated(qint32 progress);
    void decrypt_updated(qint32 progress);
    void file_done() const;
};

#endif // FILESENCRYPT_H
