#include "TestSecureMemBlock.h"
#include "SecureMemBlock.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#endif


int TestSecureMemBlock::runTests(){
	TestSecureMemBlock *test = new TestSecureMemBlock;
	int res = QTest::qExec(test);
	delete test;
	return res;
}

void TestSecureMemBlock::init()
{}

void TestSecureMemBlock::cleanup()
{}

void TestSecureMemBlock::shouldWork(){
	SecureMemBlock block(reinterpret_cast<const unsigned char*>("12345678901234561234567890123456"), 32, false);

#ifdef Q_OS_WIN
	unsigned char encrypted[32];
	memcpy(&encrypted, block.getDataNoAction(), 32);
#else
	unsigned char encrypted[48];
	memcpy(&encrypted, block.getDataNoAction(), 48);
#endif

	QVERIFY(memcmp(block.getDataNoAction(), "12345678901234561234567890123456", 32) != 0);

	qDebug() << block.getData();
	QVERIFY(memcmp(block.getData(), "12345678901234561234567890123456", 32) == 0);
	QVERIFY(memcmp(block.getDataNoAction(), "12345678901234561234567890123456", 32) == 0);
	block.secure();
	QVERIFY(memcmp(block.getData(), "12345678901234561234567890123456", 32) == 0);
	block.secure();
	QVERIFY(memcmp(block.getDataNoAction(), "12345678901234561234567890123456", 32) != 0);

	SecureMemBlock hello(reinterpret_cast<const unsigned char*>("Hello"), 5, false);
	QVERIFY(memcmp(hello.getData(), "Hello", 5) == 0);

#ifdef Q_OS_WIN
QVERIFY(memcmp(block.getDataNoAction(), &encrypted, 32) == 0);
#else
QVERIFY(memcmp(block.getDataNoAction(), &encrypted, 48) == 0);
#endif

#ifdef Q_OS_WIN
	unsigned char a[16] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
	CryptProtectMemory(a, CRYPTPROTECTMEMORY_BLOCK_SIZE, CRYPTPROTECTMEMORY_SAME_PROCESS);
	SecureMemBlock block2(a, 16, true);
	QVERIFY(memcmp(block2.getData(), "Hello", 5) == 0);
#endif
}
