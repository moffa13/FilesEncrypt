#include <QApplication>
#include "MainWindow.h"
#include <QMutexLocker>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    MainWindow m;

    return a.exec();
}
