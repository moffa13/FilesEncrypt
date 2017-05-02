#include <QMessageBox>
#include <QCloseEvent>
#include "TrollDialog.h"
#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"


SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent), m_refuseClose{false},
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    setWindowTitle("Paramètres");
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::on_checkBox_toggled(bool checked)
{
    TrollDialog* d = new TrollDialog(this);
    d->show();
    m_refuseClose = true;
    setWindowFlags( ( (windowFlags() | Qt::CustomizeWindowHint)
    & ~Qt::WindowCloseButtonHint) );
    show();
}

void SettingsWindow::closeEvent(QCloseEvent *e){
    if(!m_refuseClose){
        emit closed();
    }
}
