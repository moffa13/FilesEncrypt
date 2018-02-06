QT += core gui widgets concurrent network xml

CONFIG += c++11

RESOURCES = resources/FilesEncrypt.qrc
TRANSLATIONS = filesencrypt_en.ts

VERSION = 0.3.0
DEFINES += APP_VERSION_COMMA=0,3,0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TARGET = FilesEncrypt
CONFIG -= app_bundle
CONFIG -= console

RC_FILE = resources/FilesEncrypt.rc

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
	crypto/Crypt.cpp \
	crypto/FilesEncrypt.cpp \
	crypto/SecureMemBlock.cpp \
	network/Downloader.cpp \
	network/UpdateManager.cpp \
	ui/ChooseKey.cpp \
	ui/ContextualMenuToggler.cpp \
	ui/FilesListModel.cpp \
	ui/MainWindow.cpp \
	ui/Progress.cpp \
	ui/SettingsWindow.cpp \
	AccurateTimer.cpp \
	Init.cpp \
	Logger.cpp \
	utilities.cpp \
	Version.cpp \
        crypto/SessionKeyBase.cpp

HEADERS += \
	crypto/Crypt.h \
	crypto/FilesEncrypt.h \
	crypto/SecureMemBlock.h \
	network/Downloader.h \
	network/UpdateManager.h \
	ui/ChooseKey.h \
	ui/ContextualMenuToggler.h \
	ui/FilesListModel.h \
	ui/MainWindow.h \
	ui/Progress.h \
	ui/SettingsWindow.h \
	AccurateTimer.h \
	defines.h \
	Init.h \
	Logger.h \
	utilities.h \
	Version.h \
        crypto/SessionKey.h \
        crypto/SessionKeyBase.h

win32{
        SOURCES += crypto/SessionKey_win.cpp
        HEADERS += crypto/SessionKey_win.h

	QMAKE_CXXFLAGS += -MD
	QMAKE_CXXFLAGS_RELEASE = -O2
	INCLUDEPATH += "$$PWD/../include/windows/"
	DEPENDPATH += "$$PWD/../include/windows/"
	LIBS += -lcrypt32 -lshell32
	!contains(QMAKE_TARGET.arch, x86_64) {
		CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcrypto-x86d
		CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibssl-x86d
		CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcrypto-x86
		CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibssl-x86
	} else {
		CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcrypto-x64d
		CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibssl-x64d
		CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcrypto-x64
		CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibssl-x64
	}
}

unix{
        SOURCES += crypto/SessionKey_linux.cpp
        HEADERS += crypto/SessionKey_linux.h

	QMAKE_CXXFLAGS += -Wno-reorder -g -fno-inline -std=c++11
	INCLUDEPATH += "$$PWD/../include/linux/"
	DEPENDPATH += "$$PWD/../include/linux/"
	INCLUDEPATH += "$$PWD/../include/"
        INCLUDEPATH += /usr/include/glib-2.0/
	LIBS += -L/lib/x86_64-linux-gnu/ -lssl -lcrypto -lstdc++fs -lgcrypt
        LIBS += -L/usr/lib -lgnome-keyring
}

FORMS += \
	ui/MainWindow.ui \
	ui/Progress.ui \
	ui/ChooseKey.ui \
	ui/SettingsWindow.ui

CONFIG(debug, debug|release) {
	QT += testlib
	SOURCES += tests/TestCrypt.cpp \
		tests/TestFilesEncrypt.cpp \
		tests/TestVersion.cpp \
		tests/TestSecureMemBlock.cpp
	HEADERS += tests/TestCrypt.h \
		tests/TestFilesEncrypt.h \
		tests/TestVersion.h \
		tests/TestSecureMemBlock.h
}
