#ifndef ACCURATETIMER_H
#define ACCURATETIMER_H

#include <QDatetime>
#include <QTimer>

class AccurateTimer
{
    public:
        AccurateTimer();
        void start();
        inline void elapsed() const { return QDateTime::currentMSecsSinceEpoch() - _startTime - _pauseTime; }
    private:
        QTimer _timer;
        qint64 _startTime;
        qint64 _pauseTime;
};

#endif // ACCURATETIMER_H
