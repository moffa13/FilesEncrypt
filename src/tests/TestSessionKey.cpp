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

    // File to encrypt
	QFile f(QApplication::applicationDirPath() + "/test.file");
	f.open(QFile::ReadWrite);
	f.write("Hello", 5);
	f.close();

	// Generate a key
	FilesEncrypt::genKey(QApplication::applicationDirPath() + "/test.key", "12345", nullptr);

	MainWindow w;
	w.m_filesEncrypt = new FilesEncrypt((QApplication::applicationDirPath() + "/test.key").toStdString());
	w.m_filesEncrypt->requestAesDecrypt("12345");
	w.keySelected();
	w.beSureKeyIsSelectedAndValid([](){}, false);


		SessionKey sessionKey(&w, ".session.key.test");
		QSignalSpy spyFinishedAction(&sessionKey, &SessionKeyBase::finishedAction);
		QSignalSpy spyKeyReady(&sessionKey, &SessionKeyBase::keyReady);

		// Check a session key has correctly been created
		QVERIFY(sessionKey.checkForSessionKey(false));
		QCOMPARE(spyKeyReady.count(), 1);
		QVERIFY(QFileInfo(QApplication::applicationDirPath() + "/.session.key.test").exists());

		// Encrypt the file with it
		QEventLoop loop;
		sessionKey.action(f.fileName(), ENCRYPT);
		QTest::qWait(1000);
		QTimer::singleShot(1, &loop, SLOT(quit()));
		loop.exec();

		// Check finished is emitted
		qDebug() << spyFinishedAction.count();
		QVERIFY(spyFinishedAction.count() > 0);
		QVERIFY(w.allTasksDone(ENCRYPT));

		// Decrypt again the file
		QEventLoop loop2;
		w.action(DECRYPT);
		QTest::qWait(1000);
		QTimer::singleShot(1, &loop2, SLOT(quit()));
		loop2.exec();

		QVERIFY(w.allTasksDone(DECRYPT));

        // Clear the file
        w.on_remove_clicked();


		SessionKey sessionKey2(&w, ".session.key.test");
		sessionKey2.checkForSessionKey(false);

		f.open(QFile::ReadWrite);
		w.m_filesEncrypt->encryptFile(&f, ENCRYPT, false);
		f.close();

        QEventLoop loop3;
		sessionKey2.action(f.fileName(), DECRYPT);

		QTest::qWait(1000);
		QTimer::singleShot(1, &loop3, SLOT(quit()));
        loop3.exec();

        f.open(QFile::ReadOnly);
		QVERIFY(f.read(5) == "Hello");
        f.close();




	f.remove();
	QFile(QApplication::applicationDirPath() + "/test.key").remove();
	QFile(QApplication::applicationDirPath() + "/.session.key.test").remove();

}
