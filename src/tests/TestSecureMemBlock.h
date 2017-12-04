#ifndef TESTSECUREMEMBLOCK_H
#define TESTSECUREMEMBLOCK_H

#include <QtTest/QTest>
#include "Crypt.h"

class TestSecureMemBlock : public QObject
{

	Q_OBJECT

	public:
		static int runTests();
	private:

	private slots:
		void init();
		void cleanup();
		void shouldWork();
};

#endif // TESTSECUREMEMBLOCK_H

