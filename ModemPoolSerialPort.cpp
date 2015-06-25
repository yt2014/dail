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



    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
           qDebug() << "Name        : " << info.portName();
           qDebug() << "Description : " << info.description();
           qDebug() << "Manufacturer: " << info.manufacturer();

           if(info.manufacturer()=="Exar Corporation")
           {
               portsInfo.append(info);
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


void CModemPoolSerialPort::close()
{
    mutex.lock();

    QIODevice::close();

    mutex.unlock();

}

void CModemPoolSerialPort::closeAll()
{
    int num = portsInfo.count();
    for(int i=0;i<num;i++)
    {
        this->setPort(portsInfo.at(i));
        this->close();
    }
}
