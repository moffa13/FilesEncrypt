#include <QMessageBox>
#include <QCloseEvent>
#include "SettingsWindow.h"
#include "ui/ui_SettingsWindow.h"


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

void SettingsWindow::closeEvent(QCloseEvent *e){
    Q_UNUSED(e);
    if(!m_refuseClose){
        emit closed();
    }
}
