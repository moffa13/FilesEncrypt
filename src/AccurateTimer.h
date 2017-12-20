#ifndef ACCURATETIMER_H
#define ACCURATETIMER_H

#include <QDateTime>
#include <QTimer>

class AccurateTimer : public QObject
{

        Q_OBJECT

    public:
        AccurateTimer();
        void start();
        inline void reset(){
            _startTime = 0;
            _lastBreakTime = 0;
            _breakTime = 0;
        }
        inline void pause() { _lastBreakTime = QDateTime::currentMSecsSinceEpoch(); }
        inline qint64 elapsed() const { return _startTime == 0 ? 0 : QDateTime::currentMSecsSinceEpoch() - _startTime - _breakTime; }
        inline qint64 getTimeStarted() const { return _startTime; }
        inline qint64 getPauseTime() const { return _breakTime; }
    private:
        qint64 _startTime = 0;
        qint64 _breakTime = 0;
        qint64 _lastBreakTime = 0;
};

#endif // ACCURATETIMER_H
