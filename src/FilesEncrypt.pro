QT += core widgets concurrent

CONFIG += c++11

TARGET = FilesEncrypt
CONFIG -= app_bundle

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

QMAKE_CFLAGS_RELEASE = -O2 -MD
QMAKE_CFLAGS_DEBUG = -MD

TEMPLATE = app

SOURCES += main.cpp \
    Crypt.cpp \
    Logger.cpp \
    utilities.cpp \
    ChooseKey.cpp \
    Progress.cpp \
    MainWindow.cpp \
    FilesEncrypt.cpp \
    Settings.cpp \
    SettingsWindow.cpp

HEADERS += \
    Crypt.h \
    Logger.h \
    utilities.h \
    ChooseKey.h \
    Progress.h \
    FilesEncrypt.h \
    MainWindow.h \
    Settings.h \
    SettingsWindow.h

win32{
    INCLUDEPATH += "$$PWD/../include/windows/"
    DEPENDPATH += "$$PWD/../include/windows/"
    !contains(QMAKE_TARGET.arch, x86_64) {
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox86MDd
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx86MDd
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox86MD
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx86MD
    } else {
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox64MDd
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx64MDd
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox64MD
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx64MD
    }
}


unix{
    INCLUDEPATH += "$$PWD/../include/linux/"
    DEPENDPATH += "$$PWD/../include/linux/"
    INCLUDEPATH += "$$PWD/../include/"
    LIBS += -L/lib/x86_64-linux-gnu/ -lssl -lcrypto -lstdc++fs
}

FORMS += \
    mainwindow.ui \
    progress.ui \
    ChooseKey.ui \
    SettingsWindow.ui
