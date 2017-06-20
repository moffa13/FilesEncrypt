#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QString>
#include <QFile>
#include <iostream>
#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>

namespace Logging {

    enum ACTION{
        ERASE = 1,
        PRINT = 2,
        PRINT_FILE = 4,
        ERROR_OUT = 8
    };

    enum LOGLEVEL{
        NOTHING = 0,
        ERROR = 1,
        WARN = 2,
        DEBUG = 3
    };

    class Logger
    {
        public:
            static inline QDateTime getDateTime(){
                return QDateTime::currentDateTime();
            }
            static inline QString getDateFormatted(){
                return Logger::getDateTime().toString("yyyy-MM-dd hh:mm:ss");
            }

            template<typename Arg>
            static void write(QTextStream &os, Arg&& arg);

            template<typename First, typename ...Rest>
            static void write(QTextStream &os, First&& f, Rest&&... rest);

            template<typename ...Rest>
            static void write(const QString &prefix, int act, Rest&&... rest);

            template<typename ...Args>
            static void error(Args&&... args);
            template<typename ...Args>
            static void warn(Args&&... args);
            template<typename ...Args>
            static void debug(Args&&... args);

            template<typename ...Args>
            static void error(ACTION act = PRINT, Args&&... args);
            template<typename ...Args>
            static void warn(ACTION act = PRINT, Args&&... args);
            template<typename ...Args>
            static void debug(ACTION act = PRINT, Args&&... args);

            template<typename ...Args>
            static void Logger::log(int log_level, int act = PRINT, Args&&... args);

            static void setLogLevel(int level){ _log_level = level; }
        private:
            static int _log_level;
    };

    template<typename ...Args>
    void Logger::debug(Args&&... args){
        debug(PRINT, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    void Logger::warn(Args&&... args){
        warn(PRINT, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    void Logger::error(Args&&... args){
        error(PRINT, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    void Logger::debug(ACTION act, Args&&... args){
        if(_log_level < DEBUG) return;
        Logger::write("Info", act, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    void Logger::warn(ACTION act, Args&&... args){
         if(_log_level < WARN) return;
        Logger::write("Warn", act, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    void Logger::error(ACTION act, Args&&... args){
        if(_log_level < ERROR) return;
        int act_int = static_cast<int>(act);
        act_int |= ERROR_OUT;
        Logger::write("ERROR", act_int, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    void Logger::log(int log_level, int act, Args&&... args){

        if(_log_level < log_level) return;

        switch (log_level) {
            case ERROR:
                error(act, std::forward<Args>(args)...);
                break;
            case WARN:
                warn(act, std::forward<Args>(args)...);
                break;
            case DEBUG:
                debug(act, std::forward<Args>(args)...);
                break;
            default:
                throw std::runtime_error{"Unknown log level"};
                break;
        }
    }

    template<typename Arg>
    void Logger::write(QTextStream &os, Arg&& arg)
    {
        os << arg;
    }

    template<typename First, typename ...Rest>
    void Logger::write(QTextStream &os, First&& f, Rest&&... rest)
    {
        os << f;
        write(os, std::forward<Rest>(rest)...);
    }

    template<typename ...Rest>
    void Logger::write(const QString &prefix, int act, Rest&&... rest)
    {

        QString string;
        QTextStream stream(&string);
        stream << Logger::getDateFormatted() << " ["+prefix+"] ";
        write(stream, std::forward<Rest>(rest)...);

        auto &os = (act & ERROR_OUT) ? std::cerr : std::cout;

        if(act & PRINT){ // Print to the console
            if(act & ERASE)
                os << "\r";
            os << string.toStdString().c_str();

            if(act & ERASE)
                os << std::flush;
            else
                os << std::endl << std::flush;
        }else{
            QFile f(qApp->applicationDirPath() + "/error.log");
            if(f.open(QIODevice::WriteOnly | QIODevice::Append)){
                f.write(string.toStdString().c_str(), string.size());
            }
            else{
                throw std::runtime_error("Impossible to write in log file");
            }
        }
    }
}
#endif // LOGGER_H
