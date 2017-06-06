#include "ChooseKey.h"
#include "ui/ui_ChooseKey.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QInputDialog>
#include <QtDebug>


ChooseKey::ChooseKey(FilesEncrypt** filesEncrypt, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_filesEncrypt(filesEncrypt),
    ui(new Ui::ChooseKey)
{
    ui->setupUi(this);
    setFixedSize(size());
}

ChooseKey::~ChooseKey()
{
    delete ui;
}

void ChooseKey::closeEvent(QCloseEvent *e){
    QDialog::closeEvent(e);
    if(m_done)
        emit keyDone();
    e->accept();
}

void ChooseKey::on_newKey_clicked()
{

    if(ui->key->text().isEmpty()){
        ui->key->setText(showInputKeyDialog());
        if(ui->key->text().isEmpty())
            return;
    }

    QFile f(ui->key->text());
    int rep;
    if(f.exists()){
        rep = QMessageBox::warning(this, "Existe dejà", "Un fichier existe déjà, voulez-vous écraser ?", QMessageBox::Yes | QMessageBox::No);
    }
    if(rep == QMessageBox::No){
        return;
    }
    bool ok;
    QString password(askPassword(true, &ok, this));
    if(ok){
        FilesEncrypt::genKey(ui->key->text(), password);
        *m_filesEncrypt = new FilesEncrypt(ui->key->text().toStdString());
        (*m_filesEncrypt)->requestAesDecrypt(password.toStdString());
        m_done = true;
        close();
    }
}

void ChooseKey::on_choose_clicked()
{
    if(ui->key->text().isEmpty())
        return;

    QFile f(ui->key->text());
    if(!f.exists()){
        QMessageBox::warning(this, "Introuvable", "La clé est introuvable", QMessageBox::Ok);
    }else{
        bool ok;
req:
        QString pass{askPassword(false, &ok, this)};
        if(ok){
            *m_filesEncrypt = new FilesEncrypt(ui->key->text().toStdString());
            if(!(*m_filesEncrypt)->requestAesDecrypt(pass.toStdString())){
                goto req;
            }
            m_done = true;
            close();
        }
    }
}

QString ChooseKey::askPassword(bool newKey, bool* okCond, QWidget* parent){
    QString password;
    do{
        if(newKey){
            password = QInputDialog::getText(parent, "Mot de passe", "Entrez un mot de passe pour sécuriser la clé", QLineEdit::Password, "", okCond);
        }else{
            password = QInputDialog::getText(parent, "Mot de passe", "Entrez le mot de passe de la clé", QLineEdit::Password, "", okCond);
        }

    }while(*okCond && password.isEmpty());

    return password;
}

QString ChooseKey::showInputKeyDialog(){
    return QFileDialog::getSaveFileName(
        this,
        "Emplacement de la clé",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        "All files (*)",
        NULL,
        QFileDialog::DontConfirmOverwrite
    );
}

void ChooseKey::on_select_clicked()
{
    ui->key->setText(showInputKeyDialog());
}

void ChooseKey::on_pushButton_clicked()
{
    bool okCond;
    QString key;
    do{
        key = QInputDialog::getText(nullptr, "Clé", "Entrez une clé AES-256 en hexadécimal (64 caractères)", QLineEdit::Normal,
                                    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
                                    &okCond);

        key.replace(QRegularExpression{"\\s+"}, ""); // Remove any spaces

    }while(okCond && key.length() != 64);

    if(okCond){
        char e[32];

        for(int i{0}; i < key.length(); i += 2){
            QString tmp{key.mid(i, 2)};
            e[i / 2] = tmp.toInt(nullptr, 16);
        }

        *m_filesEncrypt = new FilesEncrypt(&e[0]);
        m_done = okCond;
        close();
    }

}
