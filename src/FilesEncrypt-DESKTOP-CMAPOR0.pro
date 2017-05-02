QT += core widgets concurrent

CONFIG += c++11

TARGET = FilesEncrypt
CONFIG -= app_bundle

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
    SettingsWindow.cpp \
    TrollDialog.cpp

HEADERS += \
    Crypt.h \
    Logger.h \
    utilities.h \
    ChooseKey.h \
    Progress.h \
    FilesEncrypt.h \
    MainWindow.h \
    Settings.h \
    SettingsWindow.h \
    TrollDialog.h

win32{
    INCLUDEPATH += "$$_PRO_FILE_PWD_/../include/windows/"
    DEPENDPATH += "$$_PRO_FILE_PWD_/../include/windows/"
    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build")

        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox86MDd
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx86MDd
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox86MD
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx86MD

    } else {
        message("x86_64 build")
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox64MDd
        CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx64MDd
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibcryptox64MD
        CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibsslx64MD

    }
}


unix{
    INCLUDEPATH += "$$_PRO_FILE_PWD_/../include/linux/"
    DEPENDPATH += "$$_PRO_FILE_PWD_/../include/linux/"
    INCLUDEPATH += "$$_PRO_FILE_PWD_/../include/"
    LIBS += -L/lib/x86_64-linux-gnu/ -lssl -lcrypto -lstdc++fs
}

FORMS += \
    mainwindow.ui \
    progress.ui \
    ChooseKey.ui \
    SettingsWindow.ui \
    TrollDialog.ui
