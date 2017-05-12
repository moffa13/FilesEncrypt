#include "Progress.h"
#include "ui_progress.h"
#include "utilities.h"
#include <QTimer>
#include <QtDebug>
#include "Logger.h"

#define TIME_REFRESH_MS 500
#define TIME_RESET_COUNTER_SEC 10

QMutex Progress::m_mutex;
QMutex Progress::m_mutex2;

Progress::Progress(FilesEncrypt** f, QWidget *parent) :
    QWidget(parent),
    m_f(f),
    ui(new Ui::Progress)
{
    ui->setupUi(this);

    connect(*m_f, SIGNAL(encrypt_updated(qint32)), this, SLOT(progressed(qint32)));
    connect(*m_f, SIGNAL(decrypt_updated(qint32)), this, SLOT(progressed(qint32)));
    connect(*m_f, SIGNAL(file_done()), this, SLOT(addFile()));

    reset();
}

Progress::~Progress()
{
    delete ui;
}

void Progress::progressed(qint32 progress){

    m_mutex.lock();
    qint32 percent = 0;
    m_done += progress;
    m_done_tmp += progress;
    if(progress > 0){
        double ratio = (double)m_max / m_done;
        percent = (double)1000 / ratio;
    }
    ui->progress->setValue(percent);

    if(m_done >= m_max){
        QTimer::singleShot(200, [this](){
                close();
                reset();
        });
    }
    renderLabels();
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

void Progress::addFile(){
    m_mutex2.lock();
    ++m_current_file;
    renderLabels();
    m_mutex2.unlock();
}

void Progress::setFileMax(quint32 n){
    m_files_max = n;
    m_current_file = 0;
    renderLabels();
}

void Progress::renderLabels(){
    if(QDateTime::currentMSecsSinceEpoch() > m_last_update + TIME_REFRESH_MS){ // Update every TIME_REFRESH_MS
        ui->file_out_of->setText("Fichier : " + QString::number(m_current_file) + "/" + QString::number(m_files_max) + " (" +  utilities::speed_to_human(m_max) + ")");
        ui->threads_n->setText("Threads : " + QString::number((*m_f)->getPendingCrypt()));
        ui->speed->setText("Vitesse : " + utilities::speed_to_human(get_speed()) + "/s");
        m_last_update = QDateTime::currentMSecsSinceEpoch();
        qDebug() << m_done << " / " << m_max;
    }
}

quint64 Progress::get_speed(){
    auto now = QDateTime::currentMSecsSinceEpoch();

    quint64 delta_time = now - m_download_update; // Time since last update (avoid freeze, pause.. to have some impact)

    if(delta_time == 0){
        return 0;
    }

    auto old_speed = m_done_tmp / ((float)delta_time / 1000);

    if(now > m_download_update + TIME_RESET_COUNTER_SEC * 1000){
        m_download_update = now;
        m_done_tmp = 0;
    }

    return old_speed;
}

void Progress::encryptionStarted(){
    auto now = QDateTime::currentMSecsSinceEpoch();
    m_time_started = now / 1000; // never changes
    m_download_update = now;
}

void Progress::on_cancel_button_clicked()
{
    qDebug() << "cc";
}

void Progress::on_pause_button_clicked()
{
    Crypt::paused ^= 1;
    if(Crypt::paused){
        ui->pause_button->setText("Play");
    }else{
        ui->pause_button->setText("Pause");
        m_download_update = 0;
    }
}
