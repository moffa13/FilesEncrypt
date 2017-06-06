#ifndef LOGGER_H
#define LOGGER_H

#include <QDateTime>
#include <QString>
#include <QMutex>


class Logger
{
public:
    static QDateTime getDateTime();
    static QString getDateFormatted();
    static void info(QString const &message);
    static void write(QString const &prefix, QString const &message);
    static void warn(QString const &message);
    static void error(QString const &message);
private:
    static QMutex s_mutex;
};

#endif // LOGGER_H
