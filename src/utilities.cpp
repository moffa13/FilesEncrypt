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

}
