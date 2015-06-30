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
    simCardStatus = IDLE;
    mutex.unlock();
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
    mutex.lock();
    simCardStatus = status;
    mutex.unlock();
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
    mutex.lock();
    simCardStatus = IDLE;
    mutex.unlock();
    m_threadForSim->stop();
}

 void CSerialPortThread::run()
 {
     while(!stopped)
     {
         m_simCardPort->processData();
         msleep(200);
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
    SIM_status tempStatus;
    mutex.lock();
    tempStrList =  dataReceived;
    tempStatus = simCardStatus;
    mutex.unlock();

    if(tempStrList.count()!=0)
    {
        qDebug()<<"received data from serial port " << this->portName() <<" " + tempStrList.at(0);
        switch(tempStatus)
        {
           case IDLE:
        {
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
            {
                tempStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    tempStatus = ComeRing;
                }
            else if(tempStrList.at(0).contains("Call Ready")&&tempStrList.at(0).contains("SIM Card have insert"))
                {
                    //delayMilliSeconds(500);
                    tempStatus = SimInserted;
                }
            //Call Ready

           // SIM Card have insert
        }
            break;
        case READY:
        {
            if(tempStrList.at(0).contains("ATD"))
            {
                processInfo infoToAdd;
                infoToAdd.processStatus = DialingOut;
                infoToAdd.simPort = this->portName();
                int indexofATD = tempStrList.at(0).indexOf("ATD");
                int indexofDot = tempStrList.at(0).indexOf(";");
                infoToAdd.telenumber = tempStrList.at(0).mid(indexofATD+3,indexofDot-indexofATD-3);
                m_telenumber = infoToAdd.telenumber;
                if(tempStrList.at(0).contains("OK"))
                {
                    qDebug()<<"change to dialing";
                    tempStatus = DialingOut;
                   // this->write("AT+CLCC\n");

                    mutex.lock();
                    proInfoListFromSIMs.append(infoToAdd);
                    mutex.unlock();
                }
                else
                {
                    if(tempStrList.at(0).contains("ERROR"))
                    {
                        tempStatus = DialFailed;
                        qDebug()<<"呼叫失败";
                    }
                    else
                    {
                       tempStatus = WaitForCommandResult;
                       recoverStatus = DialingOut;
                    }
                }
            }
            else if(tempStrList.at(0).contains("ATD")&&tempStrList.at(0).contains("ERROR")
                   )
            {
                tempStatus = DialFailed;
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
                    tempStatus = WaitForFeedBack;
                    counterRecv = 0;
                }
               // this->write("AT+CLCC\n");
            }
            else
                if(tempStrList.at(0).contains("CLCC")&&tempStrList.at(0).contains("1,0,0,0,0"))
                {
                    qDebug()<<"对方接起。。。";
                    tempStatus = WaitForFeedBack;
                }
            else
                if(tempStrList.at(0).contains("NO CARRIER"))
                {
                   // this->write("ATH\n");
                    qDebug()<<"对方挂断。。。";
                    tempStatus = WaitForFeedBack;
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
                tempStatus = IDLE;
            }
            break;
        case ComeRing:
        {
             delaySeconds(3);
            // this->write("ATH\n");
             tempStatus = WaitForFeedBack;
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
                tempStatus = IDLE;
            }
            break;
        case  SimInserted:
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
            {
                tempStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    tempStatus = ComeRing;
                }
            else if(tempStrList.at(0).contains("AT+CLIP=1")&&tempStrList.at(0).contains("OK"))
                {
                    tempStatus = NeedRegist;
                }
        break;
        case NeedRegist:
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
            {
                tempStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    tempStatus = ComeRing;
                }
            break;
         case WaitForCommandResult:
            if(tempStrList.at(0).contains("OK"))
            {
                tempStatus = recoverStatus;
                recoverStatus = READY;
            }
            else if(tempStrList.at(0).contains("ERROR"))
            {
                if(recoverStatus==DialingOut)
                   tempStatus =  DialFailed;
                recoverStatus = READY;
            }
            break;

        default:
            break;
        }
        tempStrList.clear();

        mutex.lock();
        dataReceived.removeAt(0);
        simCardStatus =  tempStatus;
        mutex.unlock();
    }

}

CSerialPortThread::CSerialPortThread(CModemPoolSerialPort*SimCardPort)
{
    m_simCardPort = SimCardPort;
}

SIM_status CModemPoolSerialPort::getSimStatus()
{
    SIM_status tempStatus;

    mutex.lock();
    tempStatus = simCardStatus;
    mutex.unlock();

     return tempStatus;

}
