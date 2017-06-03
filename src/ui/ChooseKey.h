#ifndef CHOOSEKEY_H
#define CHOOSEKEY_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>
#include "FilesEncrypt.h"

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

Q_SIGNALS:
    void keyDone();

private slots:
    void on_newKey_clicked();
    void on_choose_clicked();
    void on_select_clicked();
    void on_pushButton_clicked();

    private:
    Ui::ChooseKey *ui;
    FilesEncrypt** m_filesEncrypt;
    bool m_close = false;
    QString showInputKeyDialog();
protected:
    void closeEvent(QCloseEvent *e);
};

#endif // CHOOSEKEY_H