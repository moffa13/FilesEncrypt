QT += core gui widgets concurrent network

CONFIG += c++11

TARGET = FilesEncrypt
CONFIG -= app_bundle
CONFIG -= console

RC_FILE = FilesEncrypt.rc

Release:DESTDIR = $$PWD/../bin/release
Release:OBJECTS_DIR = $$PWD/../build/release/.obj
Release:MOC_DIR = $$PWD/../build/release/.moc
Release:RCC_DIR = $$PWD/../build/release/.rcc
Release:UI_DIR = $$PWD/../build/release/.ui

Debug:DESTDIR = $$PWD/../bin/debug
Debug:OBJECTS_DIR = $$PWD/../build/debug/.obj
Debug:MOC_DIR = $$PWD/../build/debug/.moc
Debug:RCC_DIR = $$PWD/../build/debug/.rcc
Debug:UI_DIR = $$PWD/../build/debug/.ui

TEMPLATE = app

SOURCES += main.cpp \
    Crypt.cpp \
    Logger.cpp \
    utilities.cpp \
    ui/ChooseKey.cpp \
    ui/Progress.cpp \
    ui/MainWindow.cpp \
    FilesEncrypt.cpp \
    ui/SettingsWindow.cpp \
    UpdateManager.cpp \
    AccurateTimer.cpp
HEADERS += \
    Crypt.h \
    Logger.h \
    utilities.h \
    ui/ChooseKey.h \
    ui/Progress.h \
    FilesEncrypt.h \
    ui/MainWindow.h \
    ui/SettingsWindow.h \
    UpdateManager.h \
    AccurateTimer.h

win32{
    QMAKE_CXXFLAGS += -MD
    QMAKE_CXXFLAGS_RELEASE = -O2
    INCLUDEPATH += "$$PWD/../include/windows/"
    DEPENDPATH += "$$PWD/../include/windows/"
    !contains(QMAKE_TARGET.arch, x86_64) {
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox86MDd
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx86MDd
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox86MD
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx86MD
    } else {
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcrypto-x64d
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibssl-x64d
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcrypto-x64
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibssl-x64
    }
}

unix{
    QMAKE_CXXFLAGS += -Wno-reorder
    INCLUDEPATH += "$$PWD/../include/linux/"
    DEPENDPATH += "$$PWD/../include/linux/"
    INCLUDEPATH += "$$PWD/../include/"
    LIBS += -L/lib/x86_64-linux-gnu/ -lssl -lcrypto -lstdc++fs
}

FORMS += \
    ui/MainWindow.ui \
    ui/Progress.ui \
    ui/ChooseKey.ui \
    ui/SettingsWindow.ui


CONFIG(debug, debug|release) {
    QT += testlib
    SOURCES += tests/TestCrypt.cpp \
        tests/TestFilesEncrypt.cpp
    HEADERS += tests/TestCrypt.h \
        tests/TestFilesEncrypt.h
}
