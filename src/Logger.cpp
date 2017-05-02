#include "Logger.h"
#include <iostream>
#include <QDateTime>
#include <QMutexLocker>
#include <QMutex>
#include <QtDebug>

using namespace std;

QMutex Logger::m_mutex;

QDateTime Logger::getDateTime(){
    return QDateTime::currentDateTime();
}

QString Logger::getDateFormatted(){
    return Logger::getDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void Logger::write(QString const &prefix, QString const &message){
    m_mutex.lock();
    qDebug() << Logger::getDateFormatted() << " ["+prefix+"] " << message;
    m_mutex.unlock();
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
