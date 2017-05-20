#ifndef TESTCRYPT_H
#define TESTCRYPT_H

#include <QtTest/QTest>
#include "Crypt.h"

class TestCrypt : public QObject
{

    Q_OBJECT

    public:
        static void runTests();
    private:
        Crypt* m_crypt = nullptr;

    private slots:
        void init();
        void cleanup();
        void shouldDecryptAesFromFile();
        void shouldDecryptAes();
};

#endif // TESTCRYPT_H
