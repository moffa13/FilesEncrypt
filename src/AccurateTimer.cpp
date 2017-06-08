#include "AccurateTimer.h"

AccurateTimer::AccurateTimer(){}

void AccurateTimer::start(){
    if(_lastBreakTime != 0){ // There is a break
        _breakTime += QDateTime::currentMSecsSinceEpoch() - _lastBreakTime;
        return;
    }

    _startTime = QDateTime::currentMSecsSinceEpoch();
}
