#ifndef TESTVERSION_H
#define TESTVERSION_H

#include <QtTest/QTest>

class TestVersion : public QObject
{

    Q_OBJECT

    public:
        static void runTests();
    private:

    private slots:
        void init();
        void cleanup();
        void shouldCompareCorrectly();
        void shouldThrowError();
};

#endif // TESTVERSION_H
