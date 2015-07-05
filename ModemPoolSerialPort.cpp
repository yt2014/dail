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
        qDebug()<<"received " << this->portName() <<" " + tempStrList.at(0);
        processInfo infoToAdd;
        infoToAdd.simPort = this->portName();
        infoToAdd.processStatus = tempStatus;
        infoToAdd.telenumber = "";
        if(tempStrList.at(0).contains("ATH"))
        {
            if(tempStatus>READY)
            {
                infoToAdd.processStatus = READY;
                infoToAdd.telenumber = m_telenumber;
            }
        }
        else if(tempStrList.at(0).contains("Call Ready")&&tempStrList.at(0).contains("SIM Card have insert"))
        {
            //delayMilliSeconds(500);
            infoToAdd.processStatus = SimInserted;
        }
        else
        {
        switch(tempStatus)
        {
           case IDLE:
        {
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
            {

                infoToAdd.processStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    infoToAdd.processStatus = ComeRing;
                    //get the telephone nunmber here.
                }
            else if(tempStrList.at(0).contains("Call Ready")&&tempStrList.at(0).contains("SIM Card have insert"))
                {
                    //delayMilliSeconds(500);
                    infoToAdd.processStatus = SimInserted;
                }
            else if(tempStrList.at(0).contains("AT+CMGF=1")&&(!tempStrList.at(0).contains("ERROR")))
                {
                     infoToAdd.processStatus = ReadyForSendMessage;
                }

            //Call Ready

           // SIM Card have insert
        }
            break;
        case READY:
        {
            if(tempStrList.at(0).contains("ATD"))
            {

                //infoToAdd.processStatus = DialingOut;

                int indexofATD = tempStrList.at(0).indexOf("ATD");
                int indexofDot = tempStrList.at(0).indexOf(";");
                infoToAdd.telenumber = tempStrList.at(0).mid(indexofATD+3,indexofDot-indexofATD-3);
                m_telenumber = infoToAdd.telenumber;
                if(tempStrList.at(0).contains("OK"))
                {
                    qDebug()<<"change to dialing";
                    infoToAdd.processStatus = DialingOut;
                   // this->write("AT+CLCC\n");                    
                }
                else
                {
                    if(tempStrList.at(0).contains("ERROR"))
                    {
                        infoToAdd.processStatus = DialFailed;
                        qDebug()<<"呼叫失败";
                    }
                    else
                    {
                       infoToAdd.processStatus = WaitForCommandResult;
                       recoverStatus = DialingOut;
                    }
                }
            }
            else if(tempStrList.at(0).contains("ATD")&&tempStrList.at(0).contains("ERROR")
                   )
            {
                infoToAdd.processStatus = DialFailed;
                qDebug()<<"呼叫失败";
                int indexofATD = tempStrList.at(0).indexOf("ATD");
                int indexofDot = tempStrList.at(0).indexOf(";");

                m_telenumber = tempStrList.at(0).mid(indexofATD+3,indexofDot-indexofATD-3);
                infoToAdd.telenumber = m_telenumber;
            }
            else if(tempStrList.at(0).contains("AT+CMGF=1")&&(!tempStrList.at(0).contains("ERROR")))
                {
                     infoToAdd.processStatus = ReadyForSendMessage;
                }
                /*NO CARRIER*/
            /*+CLCC: 1,0,3,0,0,"13541137539",129*/
        }
            break;
        case DialingOut:
            //qDebug()<<"拨号中。。。";
            if(tempStrList.at(0).contains("CLCC")&&( tempStrList.at(0).contains("1,0,3,0,0")
                                                   ||tempStrList.at(0).contains("1,0,2,0,0")))
            {
               // delaySeconds(1);
                qDebug()<<"clcc received 拨号中。。。";
                counterRecv = counterRecv+1;
                if(counterRecv==6)
                {
                    infoToAdd.processStatus = WaitForFeedBack;
                    counterRecv = 0;
                }
               // this->write("AT+CLCC\n");
            }
            else
                if(tempStrList.at(0).contains("CLCC")&&tempStrList.at(0).contains("1,0,0,0,0"))
                {
                    qDebug()<<"对方接起。。。";
                    infoToAdd.processStatus = WaitForFeedBack;
                }
            else
                if(tempStrList.at(0).contains("NO CARRIER"))
                {
                   // this->write("ATH\n");
                    qDebug()<<"对方挂断。。。";
                    infoToAdd.processStatus = WaitForFeedBack;
                }
            break;
        case WaitForFeedBack:            
            if(tempStrList.at(0).contains("ATH")&&tempStrList.at(0).contains("OK"))
            {

                infoToAdd.processStatus = READY;
                infoToAdd.telenumber = m_telenumber;
            }
            break;
        case ComeRing:
        {
             delaySeconds(3);
            // this->write("ATH\n");
             infoToAdd.processStatus = WaitForFeedBack;
             //calculate the tele number here
             infoToAdd.telenumber = m_telenumber;
        }
            break;
        case DialFailed:
            if(tempStrList.at(0).contains("ATH")&&tempStrList.at(0).contains("OK"))
            {
                infoToAdd.processStatus = READY;
                infoToAdd.telenumber = m_telenumber;
            }
            break;
        case  SimInserted:
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
            {
                infoToAdd.processStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    infoToAdd.processStatus = ComeRing;
                }
            else if(tempStrList.at(0).contains("AT+CLIP=1")&&tempStrList.at(0).contains("OK"))
                {
                    infoToAdd.processStatus = NeedRegist;
                }
        break;
        case NeedRegist:
            if(tempStrList.at(0).contains("COPS: 0,0,")&&tempStrList.at(0).contains("OK"))
            {
                infoToAdd.processStatus = READY;
            }
            else
                if(tempStrList.at(0).contains("RING"))
                {
                    infoToAdd.processStatus = ComeRing;
                }
            break;
         case WaitForCommandResult:
            if(tempStrList.at(0).contains("OK"))
            {
                infoToAdd.processStatus = recoverStatus;
                recoverStatus = READY;
            }
            else if(tempStrList.at(0).contains("ERROR"))
            {
                if(recoverStatus==DialingOut)
                {
                   infoToAdd.processStatus =  DialFailed;
                   infoToAdd.telenumber = m_telenumber;
                }

                recoverStatus = READY;
            }
            else if(tempStrList.at(0).contains(">"))
            {
                infoToAdd.processStatus =  NeedSendContext;
                recoverStatus = READY;
            }
            break;
        case ReadyForSendMessage:
        {
            infoToAdd.telenumber = "";
            if(tempStrList.at(0).contains("AT+CMGS="))
            {
                int indexOfdot1 = tempStrList.at(0).indexOf("\"");
                int indexOfdot2 = tempStrList.at(0).indexOf("\"",indexOfdot1);
                m_ucs4StrGet.clear();
                if(!tempStrList.at(0).contains(">"))
                {

                    if(indexOfdot1==-1)
                    {
                        infoToAdd.processStatus = WaitForCommandResult;
                        recoverStatus = ReadyForSendMessage;

                    }
                    else if(indexOfdot2==-1)
                    {
                        m_ucs4StrGet += tempStrList.at(0).mid(indexOfdot1,tempStrList.at(0).length()-indexOfdot1);
                        infoToAdd.processStatus = WaitForCommandResult;
                        recoverStatus = ReadyForSendMessage;
                    }
                    else
                    {
                        m_ucs4StrGet += tempStrList.at(0).mid(indexOfdot1,indexOfdot2);
                        infoToAdd.processStatus = WaitForCommandResult;
                        recoverStatus = ReadyForSendMessage;
                    }
                }
                else
                {
                    m_ucs4StrGet += tempStrList.at(0).mid(indexOfdot1,indexOfdot2);
                    infoToAdd.processStatus = NeedSendContext;
                }
            }
        }
            break;
        case NeedSendContext:
        {
            if(tempStrList.at(0).contains("+CMGS:"))
              infoToAdd.processStatus = ReadyForSendMessage;
        }
            break;
        default:
            break;
        }
        }


        if(tempStatus!=infoToAdd.processStatus)
        {
           mutex.lock();
           proInfoListFromSIMs.append(infoToAdd);
           mutex.unlock();
        }
        else
        {
            if(infoToAdd.processStatus==WaitForFeedBack)
            {
                mutex.lock();
                proInfoListFromSIMs.append(infoToAdd);
                mutex.unlock();
            }
        }

        tempStrList.clear();

        mutex.lock();
        dataReceived.removeAt(0);
        simCardStatus = infoToAdd.processStatus;
        mutex.unlock();

        //qDebug()<<"return from port thread";

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
