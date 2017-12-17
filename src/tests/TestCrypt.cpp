#include "TestCrypt.h"
#include "Crypt.h"
#include "openssl/aes.h"
#include "FilesEncrypt.h"

int TestCrypt::runTests(){
	TestCrypt *test = new TestCrypt;
	int res = QTest::qExec(test);
	delete test;
	return res;
}

void TestCrypt::init(){
	m_crypt = new Crypt;
}

void TestCrypt::cleanup(){
	delete m_crypt;
	m_crypt = nullptr;
}

void TestCrypt::shouldDecryptAesFromFile(){
	QFile f{QApplication::applicationDirPath() + "/test.uncrypted"};
	QFile fc{QApplication::applicationDirPath() + "/test.crypted"};
	if(!f.open(QFile::ReadWrite) || !fc.open(QFile::ReadWrite)) QFAIL("Cannot create tests files");

	f.write("hello from the other side", 25);
	f.seek(0);

	unsigned char* aes = reinterpret_cast<unsigned char*>(malloc(32));
	unsigned char* iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));

	m_crypt->genAES(AESSIZE::S256, aes);
	m_crypt->genRandomIV(iv);

	m_crypt->aes_crypt(&f, &fc, aes, iv);
	fc.open(QFile::ReadOnly);

	f.resize(0);

	m_crypt->aes_decrypt(&fc, &f, aes, iv);
	f.open(QFile::ReadOnly);

	QVERIFY(f.readAll() == QByteArray{"hello from the other side"});

	f.close();
	fc.close();

	f.remove();
	fc.remove();

	free(aes);
	free(iv);
}

void TestCrypt::shouldDecryptAes(){

	unsigned char* aes = reinterpret_cast<unsigned char*>(malloc(32));
	unsigned char* iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));
	std::string message{"hello"};
	auto encrypted_size{FilesEncrypt::getEncryptedSize(message.length())};
	unsigned char* encrypted = reinterpret_cast<unsigned char*>(malloc(encrypted_size));

	m_crypt->genAES(AESSIZE::S256, aes);
	m_crypt->genRandomIV(iv);

	m_crypt->aes_crypt(
				reinterpret_cast<const unsigned char*>(message.c_str()),
				message.length(),
				encrypted,
				aes,
				iv
	);

	QVERIFY(memcmp(message.c_str(), encrypted, message.length()) != 0);

	unsigned char* uncrypted = reinterpret_cast<unsigned char*>(malloc(encrypted_size));

	m_crypt->aes_decrypt(
				encrypted,
				encrypted_size,
				uncrypted,
				aes,
				iv
	);

	QVERIFY(memcmp(message.c_str(), uncrypted, message.length()) == 0);

	free(aes);
	free(iv);
	free(encrypted);
	free(uncrypted);

}
