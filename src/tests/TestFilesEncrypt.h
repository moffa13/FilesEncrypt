#ifndef TESTFILESENCRYPT_H
#define TESTFILESENCRYPT_H

#include <QtTest/QTest>
#include "crypto/Crypt.h"

class TestFilesEncrypt : public QObject
{

	Q_OBJECT

	public:
		static int runTests();
	private:

    private Q_SLOTS:
		void init();
		void cleanup();
		void shouldRecognizeFileState();
		void shouldEncrypt();
		void shouldReadFromFile();
		void shouldRecognizeDirState();
};

#endif // TESTFILESENCRYPT_H
