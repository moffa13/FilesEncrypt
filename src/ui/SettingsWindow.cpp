#include <QMessageBox>
#include <QCloseEvent>
#include <QString>
#include <QCheckBox>
#include <QPair>
#include "SettingsWindow.h"
#include "ui/ui_SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent), m_refuseClose{false},
    ui(new Ui::SettingsWindow),
    m_settings(new QSettings)
{
    ui->setupUi(this);

    setWindowTitle("Paramètres");

    QMap<QString, QPair<QString, QVariant>> checkNames;
    checkNames.insert("encrypt_filenames", QPair<QString, QVariant>{"Crypter les noms de fichiers", true});

    QSet<QCheckBox*> boxes;

    // Add checkboxes to a QSet and set their correct checked values regarding the config
    for(QMap<QString, QPair<QString, QVariant>>::iterator it{checkNames.begin()}; it != checkNames.end(); ++it){
        QCheckBox *box = new QCheckBox{it.value().first, this};
        const QString key{it.key()};
        connect(box, &QCheckBox::toggled, [this, key](bool checked){
            m_settings->setValue(key, checked);
        });
        box->setChecked(m_settings->value(it.key(), it.value().second).toBool());
        boxes.insert(box);
    }

    for(QSet<QCheckBox*>::iterator it{boxes.begin()}; it != boxes.end(); ++it){
        layout()->addWidget(*it);
    }
}

SettingsWindow::~SettingsWindow(){
    delete m_settings;
    delete ui;
}

void SettingsWindow::closeEvent(QCloseEvent *e){
    Q_UNUSED(e);
    if(!m_refuseClose){
        emit closed();
    }
}
