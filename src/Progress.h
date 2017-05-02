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

private:
    Ui::Progress *ui;
    qint64 m_max = 0;
    qint64 m_done = 0;
    FilesEncrypt** m_f;
    void reset();
    static QMutex m_mutex;

private Q_SLOTS:
    void progressed(qint32);
    void on_pushButton_2_clicked();
};

#endif // Progress_H
