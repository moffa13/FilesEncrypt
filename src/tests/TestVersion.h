#ifndef TESTVERSION_H
#define TESTVERSION_H

#include <QtTest/QTest>

class TestVersion : public QObject
{

    Q_OBJECT

    public:
        static int runTests();
    private:

private Q_SLOTS:
    void init();
    void cleanup();
    void shouldCompareCorrectly();
    void shouldThrowError();
};

#endif // TESTVERSION_H
