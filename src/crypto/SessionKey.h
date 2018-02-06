#ifndef SESSIONKEY_H
#define SESSIONKEY_H

#include <QtGlobal>

class SessionKey;

#ifdef Q_OS_WIN
#include "SessionKey_win.h"
#else
#include "SessionKey_linux.h"
#endif

#endif // SESSIONKEY_H
