#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &w, SLOT(appClosed()));
    return a.exec();
}
