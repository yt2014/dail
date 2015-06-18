#include "ModemPoolSerialPort.h"
#include <QTime>
#include <QDebug>

CModemPoolSerialPort * CModemPoolSerialPort::_Instance = NULL;


CModemPoolSerialPort::CModemPoolSerialPort()
{
   // this->setBaudRate();
   // this->setDataBits();

    qDebug()<<"databits is "+QString::number(this->dataBits());
    qDebug()<<"baudRate is " + QString::number(this->baudRate());

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
           qDebug() << "Name        : " << info.portName();
           qDebug() << "Description : " << info.description();
           qDebug() << "Manufacturer: " << info.manufacturer();

           // Example use QSerialPort
           QSerialPort serial;
           serial.setPort(info);
           delayMilliSeconds(50);
           if (serial.open(QIODevice::ReadWrite))
           {
               delayMilliSeconds(50);
               serial.close();
           }
       }

}

CModemPoolSerialPort *CModemPoolSerialPort::getInstance()
{
    if(_Instance==NULL)
    {
        _Instance = new CModemPoolSerialPort();
    }

    return _Instance;
}


void delaySeconds(int n)
{
    QTime startTime = QTime::currentTime();

    QTime NowTime = QTime::currentTime();
    while(startTime.secsTo(NowTime)<n)
    {
       NowTime = QTime::currentTime();
    }
}


void delayMilliSeconds(int n)
{
    QTime startTime = QTime::currentTime();
    QTime NowTime = QTime::currentTime();
    while(startTime.msecsTo(NowTime)<n)
    {
         NowTime = QTime::currentTime();
    }
}
