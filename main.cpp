#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setStyle("macintosh");
    QApplication::setStyle("macintosh");

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC","Dail");
    db.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=dail.mdb");
    db.setPassword("yt_2015_sdh");



    MainWindow w;
    w.show();

    return a.exec();
}
