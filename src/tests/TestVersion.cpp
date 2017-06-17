#include "TestVersion.h"
#include "Version.h"

void TestVersion::runTests(){
    TestVersion *test = new TestVersion;
    QTest::qExec(test);
    delete test;
}

void TestVersion::init(){

}

void TestVersion::cleanup(){

}

void TestVersion::shouldThrowError(){
    QVERIFY_EXCEPTION_THROWN(Version{"lol"}, std::runtime_error);
    QVERIFY_EXCEPTION_THROWN(Version{"1."}, std::runtime_error);
}


void TestVersion::shouldCompareCorrectly(){
    Version v{"2"};
    Version v2{"2.4.9"};
    Version v3{"2.0.0"};
    Version v4{2, 0, 0};

    QVERIFY(v < v2);
    QVERIFY(v2 > v);
    QVERIFY(v != v2);

    QVERIFY(v == v3);
    QVERIFY(v == v4);
    QVERIFY(v3 == v4);
    QVERIFY(v3 == v);

    QVERIFY(v2.getMajor() == 2);
    QVERIFY(v2.getMinor() == 4);
    QVERIFY(v2.getPatch() == 9);
    QVERIFY(v.getMajor() == 2);
    QVERIFY(v.getMinor() == 0);
    QVERIFY(v.getPatch() == 0);

}
