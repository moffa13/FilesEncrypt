#include <cstdlib>
#include <string>
#include <QString>
#include <fstream>
#include "Logger.h"
#include <experimental/filesystem>

namespace utilities{

    using namespace std;

    bool checkFileExists(string const &filename){
        return std::experimental::filesystem::exists(filename);
    }

    int random(void *){
        return rand();
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
