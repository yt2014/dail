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
    counterRecv = 0;
    connect(this,SIGNAL(readyRead()),this,SLOT(receiveData()));
    m_threadForSim = new CSerialPortThread(this);
    m_threadForSim->start();
  //  this->write("AT+CLIP=1\n");
  //  delayMilliSeconds(100);
  //  this->write("AT+COPS?\n");
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
        qDebug()<<"received data from serial port " << this->portName() <<" " + tempStrList.at(0);
        switch(simCardStatus)
        {
           case IDLE:
        {
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
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
                qDebug()<<"change to dialing";
                simCardStatus = DialingOut;
               // this->write("AT+CLCC\n");
                processInfo infoToAdd;
                infoToAdd.processStatus = DialingOut;
                infoToAdd.simPort = this->portName();
                int indexofATD = tempStrList.at(0).indexOf("ATD");
                int indexofDot = tempStrList.at(0).indexOf(";");
                infoToAdd.telenumber = tempStrList.at(0).mid(indexofATD+3,indexofDot-indexofATD-3);
                m_telenumber = infoToAdd.telenumber;
                mutex.lock();
                proInfoListFromSIMs.append(infoToAdd);
                mutex.unlock();
            }
            else if(tempStrList.at(0).contains("ATD")&&tempStrList.at(0).contains("ERROR")
                   )
            {
                simCardStatus = DialFailed;
                qDebug()<<"呼叫失败";
                int indexofATD = tempStrList.at(0).indexOf("ATD");
                int indexofDot = tempStrList.at(0).indexOf(";");
                m_telenumber = tempStrList.at(0).mid(indexofATD+3,indexofDot-indexofATD-3);
            }
                /*NO CARRIER*/
            /*+CLCC: 1,0,3,0,0,"13541137539",129*/
        }
            break;
        case DialingOut:
            qDebug()<<"拨号中。。。";
            if(tempStrList.at(0).contains("CLCC")&&tempStrList.at(0).contains("1,0,3,0,0"))
            {
               // delaySeconds(1);
                qDebug()<<"clcc received 拨号中。。。";
                counterRecv = counterRecv+1;
                if(counterRecv==6)
                {
                    simCardStatus = WaitForFeedBack;
                    counterRecv = 0;
                }
               // this->write("AT+CLCC\n");
            }
            else
                if(tempStrList.at(0).contains("CLCC")&&tempStrList.at(0).contains("1,0,0,0,0"))
                {
                    qDebug()<<"对方接起。。。";
                    simCardStatus = WaitForFeedBack;
                }
            else
                if(tempStrList.at(0).contains("NO CARRIER"))
                {
                   // this->write("ATH\n");
                    qDebug()<<"对方挂断。。。";
                    simCardStatus = WaitForFeedBack;
                }
            break;
        case WaitForFeedBack:            
            if(tempStrList.at(0).contains("ATH")&&tempStrList.at(0).contains("OK"))
            {
                processInfo infoToAdd;
                infoToAdd.processStatus = WaitForFeedBack;
                infoToAdd.simPort = this->portName();
                infoToAdd.telenumber = m_telenumber;
                mutex.lock();
                proInfoListFromSIMs.append(infoToAdd);
                mutex.unlock();
                simCardStatus = IDLE;
            }
            break;
        case ComeRing:
        {
             delaySeconds(3);
            // this->write("ATH\n");
             simCardStatus = WaitForFeedBack;
             processInfo infoToAdd;
             infoToAdd.processStatus = WaitForFeedBack;
             infoToAdd.simPort = this->portName();
             infoToAdd.telenumber = m_telenumber;
             mutex.lock();
             proInfoListFromSIMs.append(infoToAdd);
             mutex.unlock();
        }
            break;
        case DialFailed:
            if(tempStrList.at(0).contains("ATH")&&tempStrList.at(0).contains("OK"))
            {
                processInfo infoToAdd;
                infoToAdd.processStatus = DialFailed;
                infoToAdd.simPort = this->portName();
                infoToAdd.telenumber = m_telenumber;
                mutex.lock();
                proInfoListFromSIMs.append(infoToAdd);
                mutex.unlock();
                simCardStatus = IDLE;
            }
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

SIM_status CModemPoolSerialPort::getSimStatus()
{
    return simCardStatus;
}
