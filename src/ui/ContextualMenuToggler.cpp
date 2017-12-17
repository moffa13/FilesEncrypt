#include "ContextualMenuToggler.h"
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QMessageBox>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

bool ContextualMenuToggler::toggleCryptUncryptOptions(bool enable){
#ifdef Q_OS_WIN
	return regsvr(!enable);
#endif
	return false;
}

#ifdef Q_OS_WIN
bool ContextualMenuToggler::regsvr(bool unregister){
	QFile f{QApplication::applicationDirPath() + "/FilesEncryptShellExtension.dll"};
	if(f.exists()){
		ShellExecute(NULL, L"Open", L"regsvr32.exe", (std::wstring(L"FilesEncryptShellExtension.dll /s") + (unregister ? L" /u" : L"")).c_str(), NULL, SW_HIDE);
	}else{
		QMessageBox::critical(nullptr, tr("Erreur"), "Impossible de trouver FilesEncryptShellExtension.dll");
		return false;
	}
	return true;
}
#endif

