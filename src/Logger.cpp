#include "Logger.h"
#include <iostream>
#include <QDateTime>
#include <QMutexLocker>
#include <QMutex>
#include <QtDebug>

using namespace std;

QMutex Logger::s_mutex;

QDateTime Logger::getDateTime(){
    return QDateTime::currentDateTime();
}

QString Logger::getDateFormatted(){
    return Logger::getDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void Logger::write(QString const &prefix, QString const &message){
    QMutexLocker locker{&s_mutex};
    qDebug() << Logger::getDateFormatted() << " ["+prefix+"] " << message;
}

void Logger::info(QString const &message){
    Logger::write("Info", message);
}

void Logger::warn(QString const &message){
    Logger::write("Warn", message);
}

void Logger::error(QString const &message){
    Logger::write("ERROR", message);
}
