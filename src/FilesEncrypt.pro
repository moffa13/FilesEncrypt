QT += core gui widgets concurrent network xml

CONFIG += c++11

RESOURCES = FilesEncrypt.qrc
TRANSLATIONS = filesencrypt_en.ts

VERSION = 0.3.0
DEFINES += APP_VERSION_COMMA=0,3,0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

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
	utilities.cpp \
	ui/ChooseKey.cpp \
	ui/Progress.cpp \
	ui/MainWindow.cpp \
	FilesEncrypt.cpp \
	ui/SettingsWindow.cpp \
	network/UpdateManager.cpp \
	network/Downloader.cpp \
	AccurateTimer.cpp \
	Version.cpp \
	Logger.cpp \
	ui/FilesListModel.cpp \
	SecureMemBlock.cpp \
	ui/ContextualMenuToggler.cpp \
	Init.cpp

HEADERS += \
	Crypt.h \
	Logger.h \
	utilities.h \
	ui/ChooseKey.h \
	ui/Progress.h \
	FilesEncrypt.h \
	ui/MainWindow.h \
	ui/SettingsWindow.h \
	network/UpdateManager.h \
	network/Downloader.h \
	AccurateTimer.h \
	Version.h \
	defines.h \
	ui/FilesListModel.h \
	SecureMemBlock.h \
	ui/ContextualMenuToggler.h \
	Init.h

win32{
	SOURCES += SessionKey.cpp

	HEADERS += SessionKey.h

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
	QMAKE_CXXFLAGS += -Wno-reorder -g -fno-inline -std=c++11
	INCLUDEPATH += "$$PWD/../include/linux/"
	DEPENDPATH += "$$PWD/../include/linux/"
	INCLUDEPATH += "$$PWD/../include/"
	LIBS += -L/lib/x86_64-linux-gnu/ -lssl -lcrypto -lstdc++fs -lgcrypt
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
