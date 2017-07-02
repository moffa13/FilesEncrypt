#ifndef TESTFILESENCRYPT_H
#define TESTFILESENCRYPT_H

#include <QtTest/QTest>
#include "Crypt.h"

class TestFilesEncrypt : public QObject
{

    Q_OBJECT

    public:
        static int runTests();
    private:

    private slots:
        void init();
        void cleanup();
        void shouldRecognizeFileState();
        void shouldEncrypt();
        void shouldReadFromFile();
};

#endif // TESTFILESENCRYPT_H
