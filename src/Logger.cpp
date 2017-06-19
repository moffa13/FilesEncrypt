#include "Logger.h"
#include <iostream>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

using namespace std;

namespace Logging {

    int Logger::_log_level = DEBUG;

    void Logger::debug(QString const &message, int act){
        Logger::write("Info", act, message);
    }

    void Logger::warn(QString const &message, int act){
        Logger::write("Warn", act, message);
    }

    void Logger::error(QString const &message, int act){
        act |= ERROR_OUT;
        Logger::write("ERROR", act, message);
    }

    void Logger::log(int log_level, const QString &message, int act){

        if(_log_level < log_level) return;

        switch (log_level) {
            case ERROR:
                error(message, act);
                break;
            case WARN:
                warn(message, act);
                break;
            case DEBUG:
                debug(message, act);
                break;
            default:
                throw std::runtime_error{"Unknown log level"};
                break;
        }
    }

}
