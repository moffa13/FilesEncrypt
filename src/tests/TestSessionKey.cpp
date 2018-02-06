#define UNIT_FRIEND friend class TestSessionKey;

#include "TestSessionKey.h"
#include "crypto/FilesEncrypt.h"
#include "ui/MainWindow.h"
#include "crypto/SessionKey.h"
#include <QSignalSpy>

int TestSessionKey::runTests(){
    TestSessionKey *test = new TestSessionKey;
    int res = QTest::qExec(test);
    delete test;
    return res;
}

void TestSessionKey::init(){

}

void TestSessionKey::cleanup(){

}

void TestSessionKey::shouldEncryptDecryptWithSessionKey(){

    // File to crypt
    QFile f(QApplication::applicationDirPath() + "/test.file");
    f.open(QFile::ReadWrite);
    f.write("Hello", 5);
    f.close();

    // Generate a key
    FilesEncrypt::genKey(QApplication::applicationDirPath() + "/test.key", "12345", nullptr);

    MainWindow w;
    w.show();
    w.m_filesEncrypt = new FilesEncrypt((QApplication::applicationDirPath() + "/test.key").toStdString());
    w.m_filesEncrypt->requestAesDecrypt("12345");
    w.beSureKeyIsSelectedAndValid([](){}, false);

    {
        SessionKey sessionKey(&w, ".session.key.test");
        QSignalSpy spyKeyReady(&sessionKey, SIGNAL(keyReady()));
        QSignalSpy spyFinishedAction(&sessionKey, SIGNAL(finishedAction()));

        // Check a session key has correctly been created
        sessionKey.checkForSessionKey(false);
        QVERIFY(QFileInfo(QApplication::applicationDirPath() + "/.session.key.test").exists());
        QCOMPARE(spyKeyReady.count(), 1);

        // Encrypt the file with it
        QEventLoop loop;
        sessionKey.action(f.fileName(), ENCRYPT);
        QTest::qWait(1000);
        QTimer::singleShot(1, &loop, SLOT(quit()));
        loop.exec();

        // Check finished is emitted
        QVERIFY(spyFinishedAction.count() > 0);
        QVERIFY(w.allTasksDone(ENCRYPT));

        // Decrypt again the file
        QEventLoop loop2;
        w.action(DECRYPT);
        QTest::qWait(1000);
        QTimer::singleShot(1, &loop2, SLOT(quit()));
        loop2.exec();

        QVERIFY(w.allTasksDone(DECRYPT));
        // Clear the window
        w.m_filesListModel.removeLast();
    }

    {
        SessionKey sessionKey(&w, ".session.key.test");
        sessionKey.checkForSessionKey(false);

        f.open(QFile::ReadWrite);
        w.m_filesEncrypt->encryptFile(&f, ENCRYPT, false);
        f.close();

        QEventLoop loop;
        sessionKey.action(f.fileName(), DECRYPT);
        QTest::qWait(1000);
        QTimer::singleShot(1, &loop, SLOT(quit()));
        loop.exec();

        f.open(QFile::ReadOnly);
        QVERIFY(f.read(5) == "Hello");
        f.close();
    }

    f.remove();
    QFile(QApplication::applicationDirPath() + "/test.key").remove();
    QFile(QApplication::applicationDirPath() + "/.session.key.test").remove();

}
