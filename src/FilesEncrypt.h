#ifndef FILESENCRYPT_H
#define FILESENCRYPT_H

#include <QFile>
#include "Crypt.h"
#include <QDir>
#include <QStringList>
#include <QMutex>
#include "openssl/aes.h"

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
	explicit FilesEncrypt(std::string const &key_file);
	explicit FilesEncrypt(const char* aes);
    FilesEncrypt(FilesEncrypt const&) = delete;
    FilesEncrypt& operator =(FilesEncrypt const&) = delete;
    ~FilesEncrypt();
    bool encryptFile(QFile* file, EncryptDecrypt op);
    static bool genKey(QString const& file, QString const& password);
    bool readFromFile();
    static EncryptDecrypt guessEncrypted(QFile& f);
    static EncryptDecrypt guessEncrypted(QDir& dir);
    static EncryptDecrypt guessEncrypted(QByteArray const& content);
    static FilesAndSize getFilesFromDirRecursive(QDir const& dir);
    bool requestAesDecrypt(std::string const& password, bool* passOk = NULL);
	bool isAesDecrypted() const;
    const unsigned char* getAES() const;
    void setAES(const char* aes);
	void unsetAES();
    static unsigned getPendingCrypt();
    static constexpr char compare[] = {'E', 0x31, 'N', 0x31, 'C', 0x31, 'R', 0x31, 'Y', 0x31, 'P', 0x31, 'T', 0x31, 'E', 0x31, 'D', 0x31};
    static constexpr size_t COMPARE_SIZE = sizeof(compare)/sizeof(*compare);
    static constexpr size_t SIZE_BEFORE_CONTENT = COMPARE_SIZE * 2 + AES_BLOCK_SIZE;

private:
    std::string m_key_file;
    unsigned char* m_aes_crypted;
	bool m_aes_decrypted_set = false;
    size_t m_aes_crypted_length;
	unsigned char* m_aes_decrypted = nullptr;
    std::string m_private_key_crypted;
    static unsigned m_pendingCrypt;
    static QMutex m_mutex;
    void init();
    void addPendingCrypt();
    void removePendingCrypt();
    void startDeleteAesTimer();
Q_SIGNALS:
    void encrypt_updated(qint32 progress);
    void decrypt_updated(qint32 progress);
    void file_done();
};

#endif // FILESENCRYPT_H
