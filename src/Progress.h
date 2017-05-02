#ifndef Progress_H
#define Progress_H

#include <QWidget>
#include "FilesEncrypt.h"
#include <QMutex>

namespace Ui {
class Progress;
}

class Progress : public QWidget
{
    Q_OBJECT

public:
    explicit Progress(FilesEncrypt** f, QWidget *parent = 0);
    ~Progress();
    void setMax(qint64 max);
    void setFileMax(quint32 n);
    void encryptionStarted();
private:
    quint64 m_max = 0;
    quint64 m_done = 0;
    quint64 m_done_tmp = 0;
    quint64 m_speed = 0;
    quint32 m_current_file = 0;
    quint32 m_files_max = 0;
    qint64 m_time_started = 0;
    qint64 m_download_update = 0;
    qint64 m_last_update = 0;
    Ui::Progress *ui;
    FilesEncrypt** m_f;
    void reset();
    void renderLabels();
    QString speed_to_human(quint64 speed) const;
    quint64 get_speed();
    static QMutex m_mutex;
    static QMutex m_mutex2;

private Q_SLOTS:
    void progressed(qint32);
    void addFile();
    void on_pause_button_clicked();
    void on_cancel_button_clicked();
};

#endif // Progress_H
