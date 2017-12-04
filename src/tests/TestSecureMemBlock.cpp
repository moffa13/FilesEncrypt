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
	SecureMemBlock block(reinterpret_cast<const unsigned char*>("Hello"), 5, false);
	unsigned char encrypted[16];
	memcpy(&encrypted, block.getDataNoAction(), 16);

	QVERIFY(memcmp(block.getDataNoAction(), "Hello", 5) != 0);
	QVERIFY(memcmp(block.getData(), "Hello", 5) == 0);
	block.secure();
	QVERIFY(memcmp(block.getDataNoAction(), "Hello", 5) != 0);
	QVERIFY(memcmp(block.getDataNoAction(), &encrypted, 16) == 0);

#ifdef Q_OS_WIN
	unsigned char a[16] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
	CryptProtectMemory(a, CRYPTPROTECTMEMORY_BLOCK_SIZE, CRYPTPROTECTMEMORY_SAME_PROCESS);
	SecureMemBlock block2(a, 16, true);
	QVERIFY(memcmp(block2.getData(), "Hello", 5) == 0);
#endif
}
