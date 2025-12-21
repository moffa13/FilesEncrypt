#include <QFile>
#include <QFileInfo>
#include <QtDebug>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include "FilesEncrypt.h"
#include "utilities.h"
#include "openssl/pem.h"
#include "Logger.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <cassert>
#include <QApplication>
#include <QStack>
#include <openssl/rand.h>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#define TIME_MIN_REMOVE_AES 3
#define CURRENT_VERSION 2

// Current crpyts number
unsigned FilesEncrypt::s_pendingCrypt = 0;
QMutex FilesEncrypt::s_mutex;
const char FilesEncrypt::compare[] = {'E', 0x31, 'N', 0x31, 'C', 0x31, 'R', 0x31, 'Y', 0x31, 'P', 0x31, 'T', 0x31, 'E', 0x31, 'D', 0x31};
const size_t FilesEncrypt::COMPARE_SIZE = sizeof(compare)/sizeof(*compare);
const size_t FilesEncrypt::VERSION_LENGTH = 1 + 5 + 1; // V00000;
const size_t FilesEncrypt::SIZE_BEFORE_CONTENT = COMPARE_SIZE + AES_BLOCK_SIZE + VERSION_LENGTH + 1 + 8 + 256;
const char FilesEncrypt::PRIVATE_ENCRYPT_AES_SEPARATOR = 0x10;


static const char PRIV_MAGIC[] = "FILESENCRYPT_KEY";
static constexpr size_t PRIV_MAGIC_LEN = sizeof(PRIV_MAGIC) - 1;
static constexpr uint8_t PRIV_VERSION = 1;
static constexpr uint32_t PRIV_ITER = 200000; // à ajuster
static constexpr uint8_t PRIV_SALT_LEN = 16;
static constexpr uint8_t PRIV_IV_LEN   = 12;
static constexpr uint8_t PRIV_TAG_LEN  = 16;

/**
 * Constructs from a key path string
 */
FilesEncrypt::FilesEncrypt(std::string key_file) : m_key_file{std::move(key_file)}{
	init();
	m_key_file_loaded = readFromFile();
}

/**
 * Directly constructs via a decrypted aes key
 * @param aes the 32-bytes aes key
 */
FilesEncrypt::FilesEncrypt(const char* aes){
	init();
	setAES(aes);
}

/**
 * Allocates some vars and sets up the timer to delete the decrypted aes after TIME_MIN_REMOVE_AES. It does NOT start it
 * @brief FilesEncrypt::init
 */
void FilesEncrypt::init(){
	m_aes_crypted = reinterpret_cast<unsigned char*>(malloc(512));
	m_aes_decrypted = reinterpret_cast<unsigned char*>(malloc(48)); // CryptProtectMemory need multiple of 16 so it is 32 & aes needs 48 so it's 48

	m_deleteAESTimer.setSingleShot(true);
	connect(&m_deleteAESTimer, &QTimer::timeout, [this](){
		if(s_pendingCrypt == 0){
			unsetAES();
            Q_EMIT keyEncrypted();
			Logging::Logger::debug("AES key deleted from ram");
		}else{
			Logging::Logger::warn("Impossible to remove the key, already crypting/decrypting " + QString::number(s_pendingCrypt) + " file(s)");
			startDeleteAesTimer();
		}
	});
}


bool FilesEncrypt::isValidKey(QFile &f){
	if(QString::fromLocal8Bit(f.read(37)) == "-----BEGIN ENCRYPTED PRIVATE KEY-----")
		return true;
	return false;
}

FilesEncrypt::~FilesEncrypt(){
	m_deleteAESTimer.stop();
	free(m_aes_crypted);
	free(m_aes_decrypted);
}

void FilesEncrypt::addPendingCrypt(){
	QMutexLocker locker{&s_mutex};
	++s_pendingCrypt;
}

void FilesEncrypt::removePendingCrypt(){
	QMutexLocker locker{&s_mutex};
	--s_pendingCrypt;
}

unsigned FilesEncrypt::getPendingCrypt(){
	QMutexLocker locker{&s_mutex};
	return s_pendingCrypt;
}

void FilesEncrypt::setAES(const char* aes){
	m_aes_decrypted_set = true;
#ifdef Q_OS_WIN
	memcpy(m_aes_decrypted, aes, 32);
	CryptProtectMemory(m_aes_decrypted, 32, CRYPTPROTECTMEMORY_SAME_PROCESS);
#else
	SecureMemBlock block{reinterpret_cast<const unsigned char*>(aes), 32, false};
	memcpy(m_aes_decrypted, block.getDataNoAction(), block.getLen());
#endif
}

/**
 * Fills the decrypted aes with zeros
 * @brief FilesEncrypt::unsetAES
 */
void FilesEncrypt::unsetAES(){
	m_aes_decrypted_set = false;
#ifdef Q_OS_WIN
	SecureZeroMemory(m_aes_decrypted, 32);
#else
	memset(m_aes_decrypted, 0, 32);
#endif
}

SecureMemBlock FilesEncrypt::getAES() const{
	return SecureMemBlock{m_aes_decrypted, 48, true};
}

EncryptedPrivateKeyBlob FilesEncrypt::encryptPrivateKeyWithPassword(
    const QByteArray& privPem,
    const QString& password)
{
    // 1) Générer salt + iv
    QByteArray salt(PRIV_SALT_LEN, 0);
    QByteArray iv(PRIV_IV_LEN, 0);
    if (RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()), salt.size()) != 1 ||
        RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), iv.size()) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }

    // 2) Dériver la clé avec PBKDF2-HMAC-SHA256
    const int keyLen = 32; // AES-256
    QByteArray key(keyLen, 0);
    if (!PKCS5_PBKDF2_HMAC(
            password.toUtf8().constData(),
            password.toUtf8().size(),
            reinterpret_cast<unsigned char*>(salt.data()),
            salt.size(),
            PRIV_ITER,
            EVP_sha256(),
            keyLen,
            reinterpret_cast<unsigned char*>(key.data()))) {
        throw std::runtime_error("PKCS5_PBKDF2_HMAC failed");
    }

    // 3) AES-256-GCM
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    QByteArray ciphertext(privPem.size() + 16, 0); // marge
    int outLen = 0, totalLen = 0;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1 ||
        EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                           reinterpret_cast<unsigned char*>(key.data()),
                           reinterpret_cast<unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (EVP_EncryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(ciphertext.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char*>(privPem.constData()),
                          privPem.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    totalLen = outLen;

    if (EVP_EncryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(ciphertext.data()) + totalLen,
                            &outLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    totalLen += outLen;
    ciphertext.resize(totalLen);

    QByteArray tag(PRIV_TAG_LEN, 0);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, PRIV_TAG_LEN,
                            tag.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CIPHER_CTX_ctrl GET_TAG failed");
    }

    EVP_CIPHER_CTX_free(ctx);

    // 4) Construire le blob final
    QByteArray blob;
    blob.append(PRIV_MAGIC, PRIV_MAGIC_LEN);
    blob.append(char(PRIV_VERSION));

    // iter (uint32 big endian)
    blob.append(char((PRIV_ITER >> 24) & 0xFF));
    blob.append(char((PRIV_ITER >> 16) & 0xFF));
    blob.append(char((PRIV_ITER >> 8) & 0xFF));
    blob.append(char(PRIV_ITER & 0xFF));

    blob.append(char(PRIV_SALT_LEN));
    blob.append(char(PRIV_IV_LEN));

    blob.append(salt);
    blob.append(iv);
    blob.append(tag);
    blob.append(ciphertext);

    EncryptedPrivateKeyBlob out;
    out.raw = blob;
    OPENSSL_cleanse(key.data(), key.size());
    return out;
}

QByteArray FilesEncrypt::decryptPrivateKeyWithPassword(
    const EncryptedPrivateKeyBlob& blob,
    const QString& password)
{
    const QByteArray& in = blob.raw;

    if (in.size() < PRIV_MAGIC_LEN + 1 + 4 + 1 + 1)
        throw std::runtime_error("Encrypted blob too small");

    int pos = 0;
    if (memcmp(in.constData(), PRIV_MAGIC, PRIV_MAGIC_LEN) != 0)
        throw std::runtime_error("Bad magic in encrypted private key");
    pos += PRIV_MAGIC_LEN;

    uint8_t version = uint8_t(in[pos++]);
    if (version != PRIV_VERSION)
        throw std::runtime_error("Unsupported private key blob version");

    uint32_t iter = (uint8_t(in[pos]) << 24) |
                    (uint8_t(in[pos+1]) << 16) |
                    (uint8_t(in[pos+2]) << 8) |
                    uint8_t(in[pos+3]);
    pos += 4;

    uint8_t saltLen = uint8_t(in[pos++]);
    uint8_t ivLen   = uint8_t(in[pos++]);

    if (saltLen != PRIV_SALT_LEN || ivLen != PRIV_IV_LEN)
        throw std::runtime_error("Unexpected salt/iv length");

    if (pos + saltLen + ivLen + PRIV_TAG_LEN >= in.size())
        throw std::runtime_error("Encrypted blob truncated");

    QByteArray salt = in.mid(pos, saltLen); pos += saltLen;
    QByteArray iv   = in.mid(pos, ivLen);   pos += ivLen;
    QByteArray tag  = in.mid(pos, PRIV_TAG_LEN); pos += PRIV_TAG_LEN;
    QByteArray ciphertext = in.mid(pos);

    // Dériver la clé
    const int keyLen = 32;
    QByteArray key(keyLen, 0);
    if (!PKCS5_PBKDF2_HMAC(
            password.toUtf8().constData(),
            password.toUtf8().size(),
            reinterpret_cast<unsigned char*>(salt.data()),
            salt.size(),
            iter,
            EVP_sha256(),
            keyLen,
            reinterpret_cast<unsigned char*>(key.data()))) {
        throw std::runtime_error("PKCS5_PBKDF2_HMAC failed");
    }

    // Déchiffrer AES-256-GCM
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    QByteArray plaintext(ciphertext.size(), 0);
    int outLen = 0, totalLen = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr,
                           reinterpret_cast<unsigned char*>(key.data()),
                           reinterpret_cast<unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(plaintext.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                          ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }
    totalLen = outLen;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, PRIV_TAG_LEN,
                            const_cast<char*>(tag.constData())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_CTRL_GCM_SET_TAG failed");
    }

    int ret = EVP_DecryptFinal_ex(ctx,
                                  reinterpret_cast<unsigned char*>(plaintext.data()) + totalLen,
                                  &outLen);
    EVP_CIPHER_CTX_free(ctx);

    if (ret != 1) {
        throw std::runtime_error("Decryption failed (bad password or corrupted data)");
    }

    totalLen += outLen;
    plaintext.resize(totalLen);
    OPENSSL_cleanse(key.data(), key.size());
    return plaintext;
}





/**
 * Makes a 256 bits aes-cbc key encrypted with rsa by a password
 * Private key locked with a password and encrypted aes with public key are stored in the file
 * @param file The file where we write the keys
 * @param password The password which will lock the private key
 * @param aes_copy If not nullptr, instead of generating a new aes, it copies it
 * @return true if everything happend right
 */
bool FilesEncrypt::genKey(QString const& file, QString const& password, const unsigned char* aes_copy)
{
    bool success = false;
    bool error = false;

    // allocations
    unsigned char* aes = nullptr;
    unsigned char* aes_encrypted = nullptr;
    BIO* bio = nullptr;
    EVP_PKEY* keypair = nullptr;

    do {
        // Generate an RSA keypair
        keypair = Crypt::genRSA(4096);


        bio = BIO_new(BIO_s_mem());
        if (!bio) { error = true; break; }

        // Write rsa in pem format into bio
        if (!PEM_write_bio_PrivateKey(bio, keypair, nullptr, nullptr, 0, nullptr, nullptr)) {
            error = true;
            break;
        }

        // Write PEM into a QByteArray for further encrypting
        QByteArray privPem;
        {
            char buf[1024];
            int n = 0;
            while ((n = BIO_read(bio, buf, sizeof(buf))) > 0) {
                privPem.append(buf, n);
            }
        }
        BIO_free(bio);
        bio = nullptr;

        // Encrypt the pem into a blob
        EncryptedPrivateKeyBlob encBlob =
            FilesEncrypt::encryptPrivateKeyWithPassword(privPem, password);

        // Allocate mem for AES key
        aes = (unsigned char*)malloc(AESSIZE::S256);
        if (!aes) { error = true; break; }

        if (!aes_copy)
            Crypt::genAES(AESSIZE::S256, aes);
        else
            memcpy(aes, aes_copy, AESSIZE::S256);

        size_t cryptedSize = 0;
        Crypt::encrypt(keypair, aes, AESSIZE::S256, nullptr, cryptedSize);

        // Wrap AES into RSA
        aes_encrypted = (unsigned char*)malloc(cryptedSize);
        if (!aes_encrypted) { error = true; break; }

        Crypt::encrypt(keypair, aes, AESSIZE::S256, aes_encrypted, cryptedSize);


        // Écrire le fichier
        QFile f(file);
        if (!f.open(QFile::WriteOnly)) { error = true; break; }

        quint32 privLen = encBlob.raw.size();
        // Write blob size
        f.write(reinterpret_cast<const char*>(&privLen), sizeof(privLen));
        // Write blob
        f.write(encBlob.raw.constData(), encBlob.raw.size());
        // Write crypted aes
        f.write(reinterpret_cast<const char*>(aes_encrypted), cryptedSize);
        f.close();

        success = true;

    } while (false);

    // cleanup
    free(aes);
    free(aes_encrypted);
    if (bio) BIO_free(bio);
    if (keypair) EVP_PKEY_free(keypair);
    return success && !error;
}





/**
 * Tries to retrieve the key file in m_key_file, if it doesn't exist, returns false
 * Copies the crypted aes and private key to m_aes_crypted and m_private_key_crypted
 * @brief FilesEncrypt::init
 * @return True if key was retrieved, either false
 */
bool FilesEncrypt::readFromFile(){
    QFile f(m_key_file.c_str());
    if (!f.exists() || !f.open(QFile::ReadOnly)) {
        Logging::Logger::error("Cannot retrieve key");
        f.close();
        return false;
    }

    QByteArray arr{f.readAll()};
    f.close();

    if (arr.size() < 4) {
        Logging::Logger::error("Key file too small");
        return false;
    }

    const char* data = arr.constData();
    quint32 privLen = 0;
    memcpy(&privLen, data, sizeof(privLen));

    if (arr.size() < 4 + static_cast<int>(privLen)) {
        Logging::Logger::error("Key file truncated (private key blob)");
        return false;
    }

    QByteArray private_key = QByteArray::fromRawData(data + 4, privLen);
    QByteArray aes_crypted  = QByteArray::fromRawData(
        data + 4 + privLen,
        arr.size() - 4 - privLen
        );

    m_aes_crypted_length = aes_crypted.length();



    memcpy(reinterpret_cast<void*>(m_aes_crypted),
           aes_crypted.constData(),
           aes_crypted.length());

    m_private_key_crypted = private_key.toStdString();

    Logging::Logger::debug("Crypted aes and crypted private key saved");
    return true;
}


bool FilesEncrypt::isAesDecrypted() const{
	return m_aes_decrypted_set;
}

bool FilesEncrypt::requestAesDecrypt(std::string const& password, bool* passOk)
{
    bool success = false;
    BIO* bio = nullptr;
    EVP_PKEY* container = nullptr;

    if (passOk) {
        *passOk = false;
    }

    // Construire le blob à partir de m_private_key_crypted
    EncryptedPrivateKeyBlob encBlob;
    encBlob.raw = QByteArray::fromRawData(
        m_private_key_crypted.data(),
        static_cast<int>(m_private_key_crypted.size())
        );

    // 1) Déchiffrer la clé privée RSA à partir du password
    QByteArray privPem;
    try {
        privPem = FilesEncrypt::decryptPrivateKeyWithPassword(
            encBlob,
            QString::fromStdString(password)
            );
    } catch (...) {
        Logging::Logger::error("Incorrect password or corrupted private key blob");
        // passOk déjà à false
        goto cleanup;
    }

    // 2) Charger la clé privée dans un EVP_PKEY
    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        Logging::Logger::error("BIO_new failed");
        goto cleanup;
    }

    if (BIO_write(bio, privPem.constData(), privPem.length()) <= 0) {
        Logging::Logger::error("BIO_write failed");
        goto cleanup;
    }

    container = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    if (container == nullptr) {
        Logging::Logger::error("PEM_read_bio_PrivateKey failed");
        goto cleanup;
    }

    if (passOk) {
        *passOk = true;
    }

    // 3) Déchiffrer la clé AES symétrique si pas déjà en RAM
    if (!isAesDecrypted()) {
        unsigned char aes_decrypted[32] = {0};
        size_t outlen;
        int decRes = Crypt::decrypt(
            container,
            m_aes_crypted,
            m_aes_crypted_length,
            nullptr,
            outlen
        );

        if(decRes != -1)
            decRes = Crypt::decrypt(
                container,
                m_aes_crypted,
                m_aes_crypted_length,
                aes_decrypted,
                outlen
            );

        if (decRes != -1 && outlen == AESSIZE::S256) {
            success = true;
            setAES(reinterpret_cast<const char*>(aes_decrypted));
            Q_EMIT keyDecrypted();
            Logging::Logger::debug("AES successfully decrypted");
            m_aes_decrypted_set = true;
            startDeleteAesTimer();

// Effacer la clé AES en clair sur la stack
#ifdef Q_OS_WIN
            SecureZeroMemory(aes_decrypted, sizeof(aes_decrypted));
#else
            memset(aes_decrypted, 0, sizeof(aes_decrypted));
#endif
        } else {
            Logging::Logger::debug("AES not successfully decrypted");
        }
    } else {
        Logging::Logger::error("AES already decrypted");
        success = true;
    }

cleanup:
    if (bio != nullptr) {
        BIO_free(bio);
    }
    if (container != nullptr) {
        EVP_PKEY_free(container);
    }
    // ⚠️ Ne pas RSA_free(private_key) ici si c'est un pointeur "emprunté" depuis EVP_PKEY

    return success;
}


void FilesEncrypt::startDeleteAesTimer(){
	m_deleteAESTimer.start(1000 * 60 * TIME_MIN_REMOVE_AES);
}


/**
 * @brief FilesEncrypt::encryptFile
 * @param file The file must be open
 * @param op
 * @param filenameNeedsEncryption
 * @return
 */
finfo_s FilesEncrypt::encryptFile(QFile* file, EncryptDecrypt op, bool filenameNeedsEncryption){

	file->seek(0);

	finfo_s result;
	result.state = op;
	result.success = false;
	result.offsetBeforeContent = 0;

	// Create a temporary file
	QTemporaryFile tmpFile;
	tmpFile.setAutoRemove(true);
	tmpFile.open();

	// The new name the file will have (absolute file name)

	QFileInfo fileInfo{file->fileName()};
	QString name{fileInfo.absoluteFilePath()};

	// Connect signals
	Crypt crypt;
	connect(&crypt, SIGNAL(aes_decrypt_updated(qint32)), this, SIGNAL(encrypt_updated(qint32)));
	connect(&crypt, SIGNAL(aes_encrypt_updated(qint32)), this, SIGNAL(decrypt_updated(qint32)));

	EncryptDecrypt_s fileState{guessEncrypted(*file)};

    if(fileState.state == ENCRYPT && fileState.version != 2){
        /*
        // This is bad because we update the progress handler
        // Telling it that we decrypted some bytes (the file) for it to close
        // But result is used but should not becuase the  fileState version is supposedly bad
        Q_EMIT decrypt_updated(file->size() - result.offsetBeforeContent);
        Q_EMIT file_done();
        result.state = ENCRYPT;
        return result;
        */
        Logging::Logger::warn("The file version is unknown");
    }

	// If there is no action to do to the file
	if(op == fileState.state){
		Logging::Logger::warn("Trying to encrypt/decrypt a file maybe already encrypted/uncrypted");
		goto end;
	}

	// Tell the class that one more object is being used
	addPendingCrypt();


	if(op == EncryptDecrypt::ENCRYPT){

		file->seek(0);

		// Gen IV
		unsigned char* iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));
		Crypt::genRandomIV(iv);


		QString nameWithoutPath{fileInfo.fileName()};

		unsigned char* encrypted_filename = nullptr;

		if(filenameNeedsEncryption){

			{
				QString newName;

				do{
					newName = "/" + utilities::randomString(15);
				}while(QFile::exists(fileInfo.absolutePath() + newName + ".filesencrypt"));

				name = fileInfo.absolutePath() + newName + ".filesencrypt";
			}

			encrypted_filename = reinterpret_cast<unsigned char*>(malloc(getEncryptedSize(nameWithoutPath.length() * 2)));
			crypt.aes_crypt(reinterpret_cast<const unsigned char*>(nameWithoutPath.toStdU16String().c_str()), nameWithoutPath.length() * 2, encrypted_filename, getAES().getData(), iv);
		}

		// Add Header
		auto blob = getEncryptBlob(reinterpret_cast<char*>(iv), CURRENT_VERSION, filenameNeedsEncryption, reinterpret_cast<const char*>(encrypted_filename), getEncryptedSize(nameWithoutPath.length() * 2));
		result.offsetBeforeContent = blob.size();

		QByteArray fileContentEncrypted{blob};
		tmpFile.write(fileContentEncrypted);

		free(encrypted_filename);

		// Save final size
		quint64 futureSize = getEncryptedSize(file->size()) + blob.size();
		result.size = futureSize;

		// Crypt data
		crypt.aes_crypt(file, &tmpFile, getAES().getData(), iv);

		free(iv);
		iv = nullptr;

	}else{

		// Gen IV
		file->seek(fileState.offsetBeforeContent);
		result.size = crypt.aes_decrypt(file, &tmpFile, getAES().getData(), const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(fileState.iv.constData())));

		// If there is a new file name to apply
		if(!Crypt::isAborted() && fileState.filenameChanged){
			char* uncrypted_filename = reinterpret_cast<char*>(malloc(fileState.newFilename.size()));
			auto nameSize = crypt.aes_decrypt(
						reinterpret_cast<const unsigned char*>(fileState.newFilename.constData()),
						fileState.newFilename.size(),
						reinterpret_cast<unsigned char*>(uncrypted_filename),
						getAES().getData(),
						const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(fileState.iv.constData()))
			);

			QByteArray str(uncrypted_filename, nameSize);
            name = fileInfo.absolutePath() + "/" + QString::fromUtf16(reinterpret_cast<const char16_t*>(str.constData()), nameSize / 2);
			free(uncrypted_filename);
		}
	}

	result.success = !Crypt::isAborted();

	if(result.success){
		result.name = name;

		qDebug() << "Future name : " << result.name;
		qDebug() << "Future size : " << result.size;
		qDebug() << "Future state : " << result.state;


		file->remove();
		tmpFile.copy(name);
	}else{
		result.state = fileState.state;
	}

	removePendingCrypt();
    Q_EMIT file_done();
end:
	return result;
}

EncryptDecrypt_s FilesEncrypt::guessEncrypted(QFile& file){
	QByteArray content = file.read(SIZE_BEFORE_CONTENT);
	return FilesEncrypt::guessEncrypted(content);
}

size_t FilesEncrypt::getEncryptedSize(int message_length){
	return (message_length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
}

EncryptDecrypt FilesEncrypt::guessEncrypted(QDir const& dir){

	unsigned crypted = 0;
	unsigned uncrypted = 0;

	FilesAndSize files = getFilesFromDirRecursive(dir);

	if(files.files.length() == 0) return DECRYPT;

    Q_FOREACH(QString const& f, files.files){
		QFile file(f);
		if(!file.open(QFile::ReadOnly)){
			;; // TODO
		}
		if(FilesEncrypt::guessEncrypted(file).state == EncryptDecrypt::ENCRYPT){
			crypted++;
		}else{
			uncrypted++;
		}

		if(crypted != 0 && uncrypted != 0){
			return PARTIAL;
		}
	}

	if(crypted == 0){
		return EncryptDecrypt::DECRYPT;
	}else{
		return EncryptDecrypt::ENCRYPT;
	}
}

FilesAndSize FilesEncrypt::getFilesFromDirRecursive(QDir const& dir){
	QDir::Filters entryFlags{QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden};
	QFileInfoList objects = dir.entryInfoList(entryFlags);
	QStringList files;
	quint64 size{0};
	QStack<QFileInfo> stack;

	for(QFileInfo const& fInfo : objects){
		stack.push(fInfo);
	}

	while(!stack.isEmpty()){
		QFileInfo info{stack.pop()};
		if(!info.isDir()){
			files.append(info.absoluteFilePath());
			size += info.size();
		}else{
			QFileInfoList newFiles{QDir{info.absoluteFilePath()}.entryInfoList(entryFlags)};
			for(QFileInfo const& file : newFiles){
				stack.push(file);
			}
		}
	}

	FilesAndSize f;
	f.size = size;
	f.files = files;

	return f;
}

QByteArray FilesEncrypt::getEncryptBlob(const char* iv, quint32 version,
                                        bool filenameChanged,
                                        const char* newFilename,
                                        int newFilename_size)
{
    if (!filenameChanged)
        newFilename_size = 0;

    QByteArray content;
    content.append(compare);
    content.append("V");
    content.append(QString::number(version).toUtf8());
    content.append(";");
    content.append(iv, AES_BLOCK_SIZE);
    content.append(QString::number(filenameChanged).toUtf8());
    content.append(QString::number(newFilename_size).toUtf8());
    content.append(";");
    content.append(newFilename, newFilename_size);
    return content;
}


EncryptDecrypt_s FilesEncrypt::guessEncrypted(QByteArray const& content){
	QByteArray header = content.mid(0, SIZE_BEFORE_CONTENT); // Be sure we check the right size

	EncryptDecrypt_s state;
	state.state = DECRYPT;
	state.version = 1;
	state.offsetBeforeContent = 0;
	state.filenameChanged = false;

	if(memcmp(header.mid(0, COMPARE_SIZE).constData(), &compare[0], COMPARE_SIZE) != 0){
		return state;
	}

	state.state = EncryptDecrypt::ENCRYPT;

	header = header.mid(COMPARE_SIZE);

	if(header.mid(AES_BLOCK_SIZE, COMPARE_SIZE) == compare){ // There is not a version
		state.iv = header.mid(0, AES_BLOCK_SIZE);
		state.offsetBeforeContent = COMPARE_SIZE * 2 + AES_BLOCK_SIZE;
	}else{
		auto ivIndex = header.indexOf(';') + 1;
		auto version{header.mid(1, ivIndex - 2).toInt()};
		state.iv = header.mid(ivIndex, AES_BLOCK_SIZE);

		header = header.mid(ivIndex + AES_BLOCK_SIZE); // Begins at filenameChanged
		state.filenameChanged = header.mid(0, 1).toInt();

		header = header.mid(1);
		auto filenameIndex = header.indexOf(';') + 1;
		auto filenameSize{header.mid(0, filenameIndex - 1).toInt()};

		header = header.mid(filenameIndex);
		state.newFilename = header.mid(0, filenameSize);
		header = header.mid(filenameIndex + filenameSize);

		state.version = version;
		state.offsetBeforeContent = COMPARE_SIZE + ivIndex + AES_BLOCK_SIZE + 1 + filenameIndex + filenameSize ;
	}


	assert(state.iv.size() == 16);
	return state;
}
