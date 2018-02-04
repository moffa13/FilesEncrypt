#include "TestFilesEncrypt.h"
#include "crypto/FilesEncrypt.h"
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

void TestFilesEncrypt::shouldRecognizeDirState(){

	QDir appDir(QApplication::applicationDirPath());
	QDir testDir(appDir.absolutePath() + "/test-dir");
	testDir.removeRecursively();
	QVERIFY(appDir.mkdir("test-dir"));

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == DECRYPT); // Empty directory

	QFile f(testDir.absoluteFilePath("file.test"));
	QVERIFY(f.open(QFile::ReadWrite));
	f.close();

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == DECRYPT); // Directory with empty file

	QVERIFY(f.open(QFile::ReadWrite));
	f.write(QByteArray("Hello"));
	f.close();

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == DECRYPT); // Directory with uncrypted file


	FilesEncrypt::genKey(QApplication::applicationDirPath() + "/key.test", "12345");
	FilesEncrypt fi{(QApplication::applicationDirPath() + "/key.test").toStdString()};
	fi.requestAesDecrypt("12345");

	QVERIFY(f.open(QFile::ReadWrite));
	finfo_s enc1 = fi.encryptFile(&f, ENCRYPT, true);
	f.close();

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == ENCRYPT); // Directory with uncrypted file

	QFile f2(testDir.absoluteFilePath("file2.test"));
	QVERIFY(f2.open(QFile::ReadWrite));
	f2.write(QByteArray("FilesEncrypt"));
	f2.close();

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == PARTIAL); // Directory with uncrypted file


	QVERIFY(f2.open(QFile::ReadWrite));
	finfo_s enc2 = fi.encryptFile(&f2, ENCRYPT, true);
	f2.close();

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == ENCRYPT); // Directory with uncrypted file

	QFile enc1F(enc1.name);
	QVERIFY(enc1F.open(QFile::ReadWrite));
	fi.encryptFile(&enc1F, DECRYPT, true);
	enc1F.close();

	QFile enc2F(enc2.name);
	QVERIFY(enc2F.open(QFile::ReadWrite));
	fi.encryptFile(&enc2F, DECRYPT, true);
	enc2F.close();

	QVERIFY(FilesEncrypt::guessEncrypted(testDir) == DECRYPT); // Directory with uncrypted file

	QFile(QApplication::applicationDirPath() + "/key.test").remove();
	testDir.removeRecursively();
}

void TestFilesEncrypt::shouldEncrypt(){

	bool encrypt_filename{true};

n_test:
	FilesEncrypt::genKey(QApplication::applicationDirPath() + "/key.test", "12345");
	FilesEncrypt f{(QApplication::applicationDirPath() + "/key.test").toStdString()};
	QVERIFY(f.requestAesDecrypt("12345"));
	QVERIFY(f.isAesDecrypted());


	QFile file{QApplication::applicationDirPath() + "/test.file"};
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
	QVERIFY(FilesEncrypt::genKey(QApplication::applicationDirPath() + "/Test.key.unit", "12345", (const unsigned char*)"12345678901234561234567890123456"));
	FilesEncrypt f{(QApplication::applicationDirPath() + "/Test.key.unit").toStdString()};
	QVERIFY(f.isFileKeyLoaded());

	bool ok = false;
	QVERIFY(f.requestAesDecrypt("12345", &ok));
	QVERIFY(ok);

	auto fa = f.getAES();
	auto data = fa.getData();
	QVERIFY(memcmp(data, "12345678901234561234567890123456", 32) == 0);

	QFile::remove("Test.key.unit");
}
