#include "ModemPoolSerialPort.h"
#include <QTime>
#include <QDebug>
#include <QMutex>

extern QMutex mutex;


CModemPoolSerialPort::CModemPoolSerialPort()
{

    qDebug()<<"databits is "+QString::number(this->dataBits());
    qDebug()<<"baudRate is " + QString::number(this->baudRate());

    this->setBaudRate(QSerialPort::Baud115200);



   /* */
    mutex.lock();
    dataReceived.clear();
    mutex.unlock();
    simCardStatus = IDLE;
   // QObject::connect(this,SIGNAL(readyRead()),this,SLOT(receiveData()));
    this->start();
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

    if(this->isOpen())
    {

       QIODevice::close();
    }

    mutex.unlock();

    this->stop();

}


void CModemPoolSerialPort::setSimCardStatus(SIM_status status)
{
    simCardStatus = status;
}


void CModemPoolSerialPort::receiveData()
{
    QByteArray data = this->readAll();
    mutex.lock();
    dataReceived.append(data);
    mutex.unlock();
}

CModemPoolSerialPort::~CModemPoolSerialPort()
{
    if(this->isOpen())
        this->close();
    if(dataReceived.count()!=0)
    {
        mutex.lock();
        dataReceived.clear();
        mutex.unlock();
    }
    simCardStatus = IDLE;
}

 void CModemPoolSerialPort::run()
 {
     while(!stopped)
     {
         processData();
     }
     stopped = false;
 }

 void CModemPoolSerialPort::stop()
 {
     stopped = true;
 }


void CModemPoolSerialPort::processData()
{
    QStringList tempStrList;
    mutex.lock();
    tempStrList =  dataReceived;
    mutex.unlock();

    if(tempStrList.count()!=0)
    {
        switch(simCardStatus)
        {
           case IDLE:
        {
            if(tempStrList.at(0).contains("CPBS")&&tempStrList.at(0).contains("OK"))
            {
                simCardStatus = READY;
            }
        }
            break;
        case READY:
        {

        }
            break;
        default:
            break;
        }
        tempStrList.clear();
        dataReceived.removeAt(0);
    }

}

