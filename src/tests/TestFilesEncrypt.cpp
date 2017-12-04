#include "TestFilesEncrypt.h"
#include "FilesEncrypt.h"
#include <QTemporaryFile>
#include <iostream>

int TestFilesEncrypt::runTests(){
	TestFilesEncrypt *test = new TestFilesEncrypt;
	int res = QTest::qExec(test);
	delete test;
	return res;
}

void TestFilesEncrypt::init(){}

void TestFilesEncrypt::cleanup(){}

void TestFilesEncrypt::shouldRecognizeFileState(){

	QByteArray iv{"1234567890123456"};

	QByteArray oldHeader;
	oldHeader.append(FilesEncrypt::compare);
	oldHeader.append(iv); // Add iv
	oldHeader.append(FilesEncrypt::compare);

	EncryptDecrypt_s guess = FilesEncrypt::guessEncrypted(oldHeader);

	QVERIFY(guess.version == 1);

	QVERIFY(guess.offsetBeforeContent == FilesEncrypt::COMPARE_SIZE + 16 + FilesEncrypt::COMPARE_SIZE);
	QVERIFY(guess.iv == iv);
	QVERIFY(guess.filenameChanged == false);
	QVERIFY(guess.state == ENCRYPT);

	// Little tricky but iv alone can be interpreted as just content so it has to recognize as uncrypted
	QVERIFY(FilesEncrypt::guessEncrypted(iv).state == DECRYPT);

	QByteArray currentHeader{FilesEncrypt::getEncryptBlob(iv.constData(), 2, true, "hello", 5)};
	currentHeader.append("Some text to encrypt");


	guess = FilesEncrypt::guessEncrypted(currentHeader);
	QVERIFY(guess.filenameChanged == true);
	QVERIFY(guess.iv == iv);
	QVERIFY(guess.newFilename == "hello");
	QVERIFY(guess.offsetBeforeContent == currentHeader.size() - 20);
	QVERIFY(guess.state == ENCRYPT);
	QVERIFY(guess.version == 2);

}

void TestFilesEncrypt::shouldEncrypt(){

	bool encrypt_filename{true};

n_test:
	FilesEncrypt::genKey("key.test", "12345");
	FilesEncrypt f{std::string{"key.test"}};
	QVERIFY(f.requestAesDecrypt("12345"));
	QVERIFY(f.isAesDecrypted());


	QFile file{"test.file"};
	file.open(QFile::ReadWrite);
	file.write("Hello", 5);

	// Write hello and re-open
	file.close();
	file.open(QFile::ReadWrite);


	finfo_s file_crypted_state{f.encryptFile(&file, ENCRYPT, encrypt_filename)};
	QVERIFY(file_crypted_state.success);
	file.close();


	QFile f2{file_crypted_state.name};
	f2.open(QFile::ReadWrite);
	f.encryptFile(&f2, DECRYPT, encrypt_filename);
	f2.close();
	file.open(QFile::ReadOnly);
	QVERIFY(memcmp(file.readAll().constData(), "Hello", 5) == 0);
	file.close();

	f2.remove();
	file.remove();
	QFile::remove("key.test");

	if(encrypt_filename){
		encrypt_filename = false;
		goto n_test;
	}

}

void TestFilesEncrypt::shouldReadFromFile(){
	QVERIFY(FilesEncrypt::genKey("Test.key.unit", "12345", (const unsigned char*)"12345678901234561234567890123456"));
	FilesEncrypt f{std::string{"Test.key.unit"}};
	QVERIFY(f.isFileKeyLoaded());

	bool ok = false;
	QVERIFY(f.requestAesDecrypt("12345", &ok));
	QVERIFY(ok);
	QVERIFY(memcmp(f.getAES().getData(), "12345678901234561234567890123456", 32) == 0);

	QFile::remove("Test.key.unit");
}
