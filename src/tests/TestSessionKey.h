#ifndef TESTSESSIONKEY_H
#define TESTSESSIONKEY_H

#include <QtTest/QTest>

class TestSessionKey : public QObject
{

	Q_OBJECT

public:
	static int runTests();
private Q_SLOTS:
	void init();
	void cleanup();
	void shouldEncryptDecryptWithSessionKey();

};

#endif // TESTSESSIONKEY_H
