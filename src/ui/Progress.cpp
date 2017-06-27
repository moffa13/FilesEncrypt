#include "Progress.h"
#include "ui/ui_Progress.h"
#include "utilities.h"
#include <QTimer>
#include <QtDebug>
#include <QMutexLocker>
#include <QMessageBox>
#include "Logger.h"

#define TIME_REFRESH_MS 500
#define TIME_RESET_COUNTER_SEC 10

QMutex Progress::s_mutex;

Progress::Progress(FilesEncrypt** f, QWidget *parent) :
    QDialog(parent),
    m_f(f),
    ui(new Ui::Progress)
{
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);

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

    QMutexLocker locker{&s_mutex};
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
    QMutexLocker locker{&s_mutex};
    ++m_current_file;
    renderLabels();
}

void Progress::setFileMax(quint32 n){
    m_files_max = n;
    m_current_file = 0;
    renderLabels();
}

void Progress::renderLabels(){
    if(QDateTime::currentMSecsSinceEpoch() > m_last_update + TIME_REFRESH_MS){ // Update every TIME_REFRESH_MS
        ui->file_out_of->setText("Fichier : " + QString::number(m_current_file) + "/" + QString::number(m_files_max) + " (" + utilities::speed_to_human(m_done) + " / " +  utilities::speed_to_human(m_max) + ")");
        ui->threads_n->setText("Threads : " + QString::number((*m_f)->getPendingCrypt()));
        ui->speed->setText("Vitesse : " + utilities::speed_to_human(get_speed()) + "/s");
        ui->timePassed->setText("Temps écoulé : " + utilities::ms_to_time(m_timer.elapsed()));
        setFixedSize(sizeHint());
        m_last_update = QDateTime::currentMSecsSinceEpoch();
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
    m_download_update = QDateTime::currentMSecsSinceEpoch();;
    m_last_update = 0;
    m_timer.reset();
    m_timer.start();
    renderLabels();
}

void Progress::on_cancel_button_clicked()
{
    reject();
}

void Progress::reject(){
    if(m_done < m_max){
        auto resp = QMessageBox::warning(this, "Arrêt", "Etes-vous sur de vouloir arrêter l'action en cours ?", QMessageBox::Yes | QMessageBox::No);
        if(resp == QMessageBox::Yes){
            Crypt::abort();
            QDialog::reject();
        }
    }else
        QDialog::reject();
}

void Progress::on_pause_button_clicked()
{
    Crypt::setPaused(Crypt::isPaused() ^ 1);
    if(Crypt::isPaused()){
        m_timer.pause();
        ui->pause_button->setText("Resume");
    }else{
        m_timer.start();
        ui->pause_button->setText("Pause");
        m_download_update = 0;
    }
}
