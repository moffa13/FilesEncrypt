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
    QVERIFY(false);
}
