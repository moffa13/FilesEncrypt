#ifndef CHOOSEKEY_H
#define CHOOSEKEY_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>
#include "crypto/FilesEncrypt.h"

namespace Ui {
class ChooseKey;
}

class ChooseKey : public QDialog
{
	Q_OBJECT

public:
	explicit ChooseKey(FilesEncrypt** filesEncrypt, QWidget *parent = 0);
	~ChooseKey();
	static QString askPassword(bool newKey, bool* okCond, QWidget* parent = 0);

	void saveAESToFile();
	Q_SIGNALS:
	void keyDone();
	void userExit(); // Called when user hit the red cross

private Q_SLOTS:
	void on_newKey_clicked();
	void on_choose_clicked();
	void on_select_clicked();
	void on_pushButton_clicked();

private:
	Ui::ChooseKey *ui;
	FilesEncrypt** m_filesEncrypt;
	bool m_done = false;
	QString showInputKeyDialog();
protected:
	void showEvent(QShowEvent *e);
	void closeEvent(QCloseEvent *e);
	void dragLeaveEvent(QDragLeaveEvent * event);
	void dragMoveEvent(QDragMoveEvent * event);
	void dropEvent(QDropEvent * event);
	void dragEnterEvent(QDragEnterEvent * event);
};

#endif // CHOOSEKEY_H
