#include "rarepattern.h"
#include <QApplication>
#include <QDebug>
#include <QHash>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RarePattern w;


    w.show();
    
    return a.exec();
}
