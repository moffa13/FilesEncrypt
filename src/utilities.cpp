#include <cstdlib>
#include <string>
#include <QString>
#include <fstream>
#include "Logger.h"
#include "openssl/err.h"
#include <experimental/filesystem>
#include <random>
#include <QtDebug>

namespace utilities{

    using namespace std;

    bool checkFileExists(string const &filename){
        return std::experimental::filesystem::exists(filename);
    }

    /**
     * Returns a number between a and b, which are included
     * @brief CloudflareScraper::random
     * @param a
     * @param b
     */
    unsigned random(unsigned a, unsigned b){
        std::mt19937 random_gen;
        random_gen.seed(std::random_device()());
        std::uniform_int_distribution<std::mt19937::result_type> rnd(a, b);
        return rnd(random_gen);
    }

    QString randomString(unsigned size){
        static constexpr char alphanum[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

        QByteArray res;

        for (unsigned i{0}; i < size; ++i) {
            auto r = random(0, sizeof(alphanum) - 2);
            res.append(alphanum[r]);
        }

        return QString::fromLocal8Bit(res);
    }

    QString speed_to_human(quint64 speed){

        QString letter;
        float speed2 = speed;

        if(speed < 1024){
            letter = "o";
        }else if(speed < 1048576){
            letter = "ko";
            speed2 = speed / 1024;
        }else if(speed < 1073741824){
            letter = "Mo";
            speed2 = speed / pow(1024, 2);
        }else if(speed < 1099511627766){
            letter = "Go";
            speed2 = speed / pow(1024, 3);
        }else{
            letter = "To";
            speed2 = speed / pow(1024, 4);
        }

        QString res = QString("%1 %2").arg(QString::asprintf("%.3f", speed2), letter);

        return res;
    }

    QString ms_to_time(int msecs){
        QString formattedTime;

        int hours{msecs / (1000 * 60 * 60)};
        int minutes{(msecs - (hours * 1000 * 60 * 60)) / (1000 * 60)};
        int seconds{(msecs - (minutes * 1000 * 60) - (hours * 1000 * 60 * 60)) / 1000};

        formattedTime.append(
            QString("%1").arg(hours, 2, 10, QLatin1Char{'0'}) + "h " +
            QString( "%1" ).arg(minutes, 2, 10, QLatin1Char{'0'}) + "m " +
            QString( "%1" ).arg(seconds, 2, 10, QLatin1Char{'0'}) + "s"
        );

        return formattedTime;
    }

    void logOpenSSLErrors()
    {
        ERR_print_errors_cb(
            [](const char* str, size_t len, void* u) -> int {
                Logging::Logger::error(QString::fromUtf8(str, int(len)).trimmed());
                return 1;
            },
            nullptr
            );
    }

}
