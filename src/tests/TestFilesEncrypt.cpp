#include "TestFilesEncrypt.h"
#include "FilesEncrypt.h"

void TestFilesEncrypt::runTests(){
    TestFilesEncrypt *test = new TestFilesEncrypt;
    QTest::qExec(test);
    delete test;
}

void TestFilesEncrypt::init(){

}

void TestFilesEncrypt::cleanup(){

}

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

void TestFilesEncrypt::shouldReadFromFile(){
    QVERIFY(FilesEncrypt::genKey("Test.key.unit", "12345"));
    FilesEncrypt f{std::string{"Test.key.unit"}};
    QVERIFY(f.isFileKeyLoaded());

    bool ok = false;
    QVERIFY(f.requestAesDecrypt("12345", &ok));
    QVERIFY(ok);

    QFile::remove("Test.key.unit");
}
