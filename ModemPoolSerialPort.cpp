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
    connect(this,SIGNAL(readyRead()),this,SLOT(receiveData()));
    m_threadForSim = new CSerialPortThread(this);
    m_threadForSim->start();
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

    m_threadForSim->stop();
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
    m_threadForSim->stop();
}

 void CSerialPortThread::run()
 {
     while(!stopped)
     {
         m_simCardPort->processData();
     }
     stopped = false;
 }

 void CSerialPortThread::stop()
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
        qDebug()<<"received data from serial port " << tempStrList.at(0);
        switch(simCardStatus)
        {
           case IDLE:
        {
            if(tempStrList.at(0).contains("AT+CLIP")&&tempStrList.at(0).contains("OK"))
            {
                simCardStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    simCardStatus = ComeRing;
                }
        }
            break;
        case READY:
        {
            if(tempStrList.at(0).contains("ATD")&&tempStrList.at(0).contains("OK"))
            {
                simCardStatus = DialingOut;
                this->write("AT+CLCC\n");
            }
                /*NO CARRIER*/
            /*+CLCC: 1,0,3,0,0,"13541137539",129*/
        }
            break;
        case DialingOut:
            if(tempStrList.at(0).contains("CLCC")&&tempStrList.at(0).contains("1,0,3,0,0"))
            {
                delaySeconds(1);
                this->write("AT+CLCC\n");
            }
            else
                if(tempStrList.at(0).contains("NO CARRIER"))
                {
                    this->write("ATH\n");
                    simCardStatus = WaitForFeedBack;
                }
            break;
        case WaitForFeedBack:
            if(tempStrList.at(0).contains("ATH")&&tempStrList.at(0).contains("OK"))
            {
                simCardStatus = IDLE;
            }
            break;
        case ComeRing:
             delaySeconds(3);
             this->write("ATH\n");
             simCardStatus = WaitForFeedBack;
            break;

        default:
            break;
        }
        tempStrList.clear();
        mutex.lock();
        dataReceived.removeAt(0);
        mutex.unlock();
    }

}

CSerialPortThread::CSerialPortThread(CModemPoolSerialPort*SimCardPort)
{
    m_simCardPort = SimCardPort;
}
