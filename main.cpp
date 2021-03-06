﻿#include "mainwindow.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QMessageBox>

#include "diskid32.h"
#include <QLibrary>
#include <QUuid>
#include <QFile>
#include <QDateTime>

typedef diskInfo (*Fun)();

QTextStream logfile;
QFile file;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setStyle("macintosh");
    QApplication::setStyle("macintosh");

        file.setFileName("log.txt");
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        logfile.setDevice(&file);
        logfile<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC","Dail");
    db.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=dail.mdb");
    db.setPassword("yt_2015_sdh");

    QString strDiskInfo;

    //read the disk number of this computer.
    QUuid id = QUuid::createUuidV3(QUuid("{1f695c5f-df58-44fb-bf28-0f5556443050}"),QString("aaaaa"));
    QString strId = id.toString();

    qDebug()<<"uuid is "<<strId;
     QLibrary mylib("diskid32.dll"); //声明所用到的dll文件
     if (mylib.load()) //判断是否正确加载
     {
        Fun getDiskInfo=(Fun)mylib.resolve("getDiskInfo"); //援引 add() 函数
        if (getDiskInfo) //是否成功连接上 add() 函数
        {
           //qDebug() << "serialNumber:" << getDiskInfo().serialNumber;

           strDiskInfo = getDiskInfo().serialNumber;
        }
        else
        {
          qDebug() << "function connect failed";
        }
     }
     else
     {
        //QMessageBox::information(NULL,"NO","DLL is not loaded!");
        qDebug() << "dll load failed";
     }


    QSqlDatabase db1 = QSqlDatabase::database("Dail",true);

     if(db1.isValid()&&db1.isOpen())
     {
         QString strQuery = "select * from verification where hdnumber like \'%" + strDiskInfo.trimmed() +"%\'";

         QSqlQuery query(db1);

         query.exec(strQuery);



         int countOfRecord = 0;
         while(query.next())
         {
             countOfRecord = countOfRecord+1;
         }
         if(countOfRecord>0)
         {
            MainWindow w;
            w.show();
            file.close();
            return a.exec();
         }
         else
         {
             QMessageBox::information(NULL,"NO","认证不通过");
         }

         db1.close();
         file.close();
         return 0;
     }
     else
     {
         QMessageBox::information(NULL,"NO","数据库文件丢失");
         file.close();
         return 0;
     }


}
