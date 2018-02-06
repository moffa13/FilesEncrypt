#ifndef TESTSECUREMEMBLOCK_H
#define TESTSECUREMEMBLOCK_H

#include <QtTest/QTest>
#include "crypto/Crypt.h"

class TestSecureMemBlock : public QObject
{

	Q_OBJECT

	public:
		static int runTests();
	private:

    private Q_SLOTS:
		void init();
		void cleanup();
		void shouldWork();
};

#endif // TESTSECUREMEMBLOCK_H

