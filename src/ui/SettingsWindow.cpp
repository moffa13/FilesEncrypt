#include <QMessageBox>
#include <QCloseEvent>
#include <QString>
#include <QCheckBox>
#include <QPair>
#include "SettingsWindow.h"
#include "ui/ui_SettingsWindow.h"

QMap<QString, QPair<QString, QVariant>> SettingsWindow::checkNames;
bool SettingsWindow::defaultValuesInit = false;

void SettingsWindow::init(){
    if(!defaultValuesInit){
        defaultValuesInit = true;
        checkNames.insert("encrypt_filenames", QPair<QString, QVariant>{"Crypter les noms de fichiers", true});
        checkNames.insert("check_beta", QPair<QString, QVariant>{"Autoriser les mises à jour beta (risqué)", false});
    }
}

SettingsWindow::SettingsWindow(QWidget *parent) :
    QDialog(parent), m_refuseClose{false},
    ui(new Ui::SettingsWindow),
    m_settings(new QSettings)
{
    ui->setupUi(this);
    ui->verticalLayout->setAlignment(Qt::AlignTop);

    setWindowTitle("Paramètres");

    init();

    // Add checkboxes to a QSet and set their correct checked values regarding the config
    for(QMap<QString, QPair<QString, QVariant>>::iterator it{checkNames.begin()}; it != checkNames.end(); ++it){
        QCheckBox *box = new QCheckBox{it.value().first, this};
        const QString key{it.key()};
        connect(box, &QCheckBox::toggled, [this, key](bool checked){
            m_settings->setValue(key, checked);
        });
        box->setChecked(m_settings->value(it.key(), getDefaultSetting(it.key())).toBool());
        ui->verticalLayout->addWidget(box);
    }

    setFixedSize(sizeHint());
}

QVariant SettingsWindow::getDefaultSetting(QString const& name){
    init();
    if(!checkNames.contains(name)) throw std::runtime_error("There is no default setting with this name");
    return checkNames[name].second;
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
