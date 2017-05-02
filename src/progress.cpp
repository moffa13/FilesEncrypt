#include "Progress.h"
#include "ui_progress.h"
#include <QTimer>
#include <QtDebug>
#include "Logger.h"

QMutex Progress::m_mutex;

Progress::Progress(FilesEncrypt** f, QWidget *parent) :
    QWidget(parent),
    m_f(f),
    ui(new Ui::Progress)
{
    ui->setupUi(this);

    connect(*m_f, SIGNAL(encrypt_updated(qint32)), this, SLOT(progressed(qint32)));
    connect(*m_f, SIGNAL(decrypt_updated(qint32)), this, SLOT(progressed(qint32)));

    reset();
}

void Progress::progressed(qint32 progress){

    m_mutex.lock();
    //Logger::info(QString::number(m_done) + " / " + QString::number(m_max));
    qint32 percent = 0;
    m_done += progress;
    if(progress > 0){
        double ratio = (double)m_max / m_done;
        percent = (double)1000 / ratio;
    }
    ui->progress->setValue(percent);

    if(m_done == m_max){
        QTimer::singleShot(1000, [this](){
            close();
            reset();
        });
    }
    m_mutex.unlock();

}

void Progress::reset(){
    ui->progress->setValue(0);
    m_done = 0;
}

void Progress::setMax(qint64 max){
    m_done = 0;
    m_max = max;
}

Progress::~Progress()
{
    delete ui;
}

void Progress::on_pushButton_2_clicked()
{
    qDebug() << m_done << m_max;
}
