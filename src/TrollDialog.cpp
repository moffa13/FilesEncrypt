#include <QtDebug>
#include <cmath>
#include "TrollDialog.h"
#include "ui_TrollDialog.h"

TrollDialog::TrollDialog(QWidget *parent, unsigned i) :
    QDialog(parent), m_i(i),
    ui(new Ui::TrollDialog)
{
    ui->setupUi(this);
}

TrollDialog::~TrollDialog()
{
    delete ui;
}

QPoint TrollDialog::randomQPoint(uint16_t a, uint16_t b){
    uint16_t a_r = rand() % (a - 364);
    uint16_t b_r = rand() % (b - 132);
    return QPoint(a_r, b_r);
}

void TrollDialog::show(){
    QDialog::show();
    qDebug() << TrollDialog::randomQPoint(1920, 1080);
    move(TrollDialog::randomQPoint(1920, 1080));
}

void TrollDialog::closeEvent(QCloseEvent *){
    for(unsigned i{0}; i < m_i; ++i){
        qDebug() << m_i + 1;
        TrollDialog* t = new TrollDialog(this, m_i + 1);
        t->show();
    }
}
