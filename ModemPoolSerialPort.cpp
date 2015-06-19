#include "ModemPoolSerialPort.h"
#include <QTime>
#include <QDebug>
#include <QMutex>

extern QMutex mutex;

CModemPoolSerialPort * CModemPoolSerialPort::_Instance = NULL;
serialPortInfoList portsInfo;

CModemPoolSerialPort::CModemPoolSerialPort()
{
   // this->setBaudRate();
   // this->setDataBits();

    qDebug()<<"databits is "+QString::number(this->dataBits());
    qDebug()<<"baudRate is " + QString::number(this->baudRate());

    this->setBaudRate(QSerialPort::Baud115200);

    portsInfo.clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
           qDebug() << "Name        : " << info.portName();
           qDebug() << "Description : " << info.description();
           qDebug() << "Manufacturer: " << info.manufacturer();


           portsInfo.append(info);

           // Example use QSerialPort
          /* QSerialPort *serial = new QSerialPort();
           serial->setPort(info);
           serial->setBaudRate(QSerialPort::Baud115200);
           ports.append(serial);


           //delayMilliSeconds(1000);
           if (serial.open(QIODevice::ReadWrite))
           {
               qDebug()<<"open com port successfully";
               delayMilliSeconds(1000);
               serial.close();
           }*/
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


void CModemPoolSerialPort::close()
{
    mutex.lock();

    QSerialPort::close();

    mutex.unlock();

}

