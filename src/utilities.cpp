#include <cstdlib>
#include <string>
#include <QString>
#include <fstream>
#include "Logger.h"
#include <experimental/filesystem>
#include <random>

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
            res.append(alphanum[random(0, sizeof(alphanum) - 1)]);
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
            speed2 = speed / pow(1024, 3);
        }

        return QString().sprintf("%.3f", speed2) + " " + letter;
    }

}
