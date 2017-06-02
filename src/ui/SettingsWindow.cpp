#include <QMessageBox>
#include <QCloseEvent>
#include <QString>
#include "SettingsWindow.h"
#include "ui/ui_SettingsWindow.h"


SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent), m_refuseClose{false},
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    setWindowTitle("Paramètres");

    QMap<QString, QString> checkNames;
    checkNames.insert("encrypt_filenames", "Crypter les noms de fichiers");

    QSet<QCheckBox*> boxes;

    for(QMap<QString, QString>::iterator it{checkNames.begin()}; it != checkNames.end(); ++it){
        boxes.insert(new QCheckBox{it.value(), this});
    }


}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::closeEvent(QCloseEvent *e){
    Q_UNUSED(e);
    if(!m_refuseClose){
        emit closed();
    }
}
