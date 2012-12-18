#include <QApplication>
#include "maincontroller.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainController c;
    c.show();

    return a.exec();
}
