#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("Memtester GUI");
    a.setOrganizationName("pyropus");

    MainWindow w;
    w.show();

    return a.exec();
}
