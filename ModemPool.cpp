#include "ModemPool.h"
#include <QDebug>

CModemPool * CModemPool::_instance = NULL;

QStringList numsNeedProcess;
processInfoList proInfoListFromSIMs; //this global list is used to synchronize the thread.
extern QMutex mutex;

CModemPool::CModemPool()
{
    CModemPoolSerialPort * tempSIMPort;
    PortSIMList.clear();
    portsInfo.clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
               qDebug() << "Name        : " << info.portName();
               qDebug() << "Description : " << info.description();
               qDebug() << "Manufacturer: " << info.manufacturer();

               if(info.manufacturer()=="Exar Corporation")
               {
                   portsInfo.append(info);

                   tempSIMPort = new CModemPoolSerialPort();
                   tempSIMPort->setPort(info);

                   PortSIMList.append(tempSIMPort);

                 //  QThread::connect(tempSIMPort,SIGNAL(readyRead()),tempSIMPort,SLOT(receiveData()));
               }
           }
    int num = PortSIMList.count();
    int i=0;
    for(i=0;i<num;i++)
    {
        PortSIMList.at(i)->open(QIODevice::ReadWrite);
        PortSIMList.at(i)->write("AT+CLIP=1\n");

    }

    connect(this,SIGNAL(needInteract()),this,SLOT(interact()));
    m_pBtn = NULL;
    m_treeWidget = NULL;
    numClicked = 0;
    m_CommRecordTable = NULL;
}

CModemPool * CModemPool::getInstance()
{
   if(_instance==NULL)
   {
       _instance = new CModemPool();
   }
   return _instance;
}

CModemPoolSerialPort * CModemPool::getSIMPort(int index)
{
    if((index>=0)&&(index<PortSIMList.count()))
    {
       return PortSIMList.at(index);
    }
    else
    {
        return NULL;
    }
}

QSerialPortInfo CModemPool::getPortInfo(int index)
{
    if((index>=0)&&(index<PortSIMList.count()))
    {
        return portsInfo.at(index);
    }
    else
    {
        return QSerialPortInfo();
    }
}

void CModemPool::scanPorts()
{
    CModemPoolSerialPort * tempSIMPort;
    PortSIMList.clear();
    portsInfo.clear();

    this->closeAllPorts();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
               qDebug() << "Name        : " << info.portName();
               qDebug() << "Description : " << info.description();
               qDebug() << "Manufacturer: " << info.manufacturer();

               if(info.manufacturer()=="Exar Corporation")
               {
                   portsInfo.append(info);

                   tempSIMPort = new CModemPoolSerialPort();
                   tempSIMPort->setPort(info);

                   PortSIMList.append(tempSIMPort);

                  // QThread::connect(tempSIMPort,SIGNAL(readyRead()),tempSIMPort,SLOT(receiveData()));
               }
           }
}

int CModemPool::portsCount()
{
    return portsInfo.count();
}

void CModemPool::closeAllPorts()
{
    int num = portsInfo.count();
    for(int i=0;i<num;i++)
    {
        PortSIMList.at(i)->close();
    }

    PortSIMList.clear();
    portsInfo.clear();
}

void CModemPool::startProcess()
{
    if(numClicked==0)
    {
        m_proInfoList.clear();
        m_teleProStepList.clear();
        int num = portsInfo.count();
        mutex.lock();
        int len = numsNeedProcess.count();
        mutex.unlock();
        int i=0;
        qDebug()<<"number of ports "<<num;
        qDebug()<<"num of tele "<<len;
        /*为所有需要处理的电话号码初始化状态 NOT_PROCESSED*/
        teleProSteps oneTeleProStepInfo;
        for(i=0;i<len;i++)
        {
             oneTeleProStepInfo.telenumber = numsNeedProcess.at(i);
             oneTeleProStepInfo.teleStep = NOT_PROCESSED;
             oneTeleProStepInfo.recordToStore.callDuration = 0;
             oneTeleProStepInfo.recordToStore.isCallIn = false;
             oneTeleProStepInfo.recordToStore.isCallConnected = 0;
             oneTeleProStepInfo.recordToStore.ringTimes = 0;
             oneTeleProStepInfo.recordToStore.startTime = QDateTime::currentDateTime();
             oneTeleProStepInfo.recordToStore.telenum = oneTeleProStepInfo.telenumber;
             m_teleProStepList.append(oneTeleProStepInfo);
        }

        /*为所有可用的SIM卡分配需处理的电话号码，并且开始处理（拨号/发信息）*/
        QString strDial;
        proInfoListFromSIMs.clear();
        for(i=0;i<num;i++)
        {
           SIM_status st = PortSIMList.at(i)->getSimStatus();
           qDebug()<<"sim"<<i<<" status:"<<st;
          if(st==READY)
          {
              if(i<len)
              {
                 strDial = "ATD"+numsNeedProcess.at(i) + ";\n";
                 QByteArray ba = strDial.toLatin1();

                 char* ch = ba.data();
                 PortSIMList.at(i)->write(ch);
               }
           }
           else if(st==IDLE)
           {
               PortSIMList.at(i)->write("AT+COPS?\n");
           }
       }

       this->start();

       numClicked = 1;
       //setting the text of btn to be "stop"
       //m_mainWindow->ui
       m_pBtn->setText("停止");
    }
    else if(numClicked==1)
    {
        this->stop();//停止拨号
        numClicked = 2;
        //setting the text to be "ok"
        m_pBtn->setText("确定");
    }
    else if(numClicked==2)
    {
       numClicked = 0;

       m_proInfoList.clear();
       m_teleProStepList.clear();

       mutex.lock();
       numsNeedProcess.clear();
       mutex.unlock();

       qDebug()<<"emit endprocess";
       emit endProcess();
        //setting the text to be "start"
       m_pBtn->setText("开始");
    }


}

void CModemPool::processStatusChange()
{

    processInfoList tempInfoList;
    mutex.lock();
    tempInfoList =  proInfoListFromSIMs;
    mutex.unlock();

    if(tempInfoList.count()!=0)
    {
       qDebug()<<"received info from SIM card";
        processInfo infoFromSIMCard = tempInfoList.at(0);

       QString telenumber = infoFromSIMCard.telenumber;
       SIM_status st = infoFromSIMCard.processStatus;

       qDebug()<<"status received is "<<st;
       QString simPort = infoFromSIMCard.simPort;

       int index = numsNeedProcess.indexOf(telenumber);//search in tele number list
       teleProSteps stepsInfoOneNum = m_teleProStepList.takeAt(index);
       if(st==DialingOut)
       {
          stepsInfoOneNum.teleStep = DAILING_OUT;
          stepsInfoOneNum.recordToStore.startTime = QDateTime::currentDateTime();
          stepsInfoOneNum.recordToStore.isCallIn = 0;
          m_teleProStepList.insert(index,stepsInfoOneNum);
          m_treeWidget->topLevelItem(index)->setText(1,"拨号中。。。");
          m_treeWidget->show();
          this->sleep(1);
       }
       else if(st==WaitForFeedBack)
       {
          stepsInfoOneNum.teleStep = PROCESS_FINISHED;
          stepsInfoOneNum.recordToStore.callDuration = stepsInfoOneNum.recordToStore.startTime.secsTo(QDateTime::currentDateTime());
          stepsInfoOneNum.recordToStore.ringTimes = stepsInfoOneNum.recordToStore.callDuration/8;
          m_teleProStepList.insert(index,stepsInfoOneNum);
          if(m_CommRecordTable!=NULL)
          {
             m_CommRecordTable->addOneRecord(stepsInfoOneNum.recordToStore);
          }
          m_treeWidget->topLevelItem(index)->setText(1,"拨号完成");
          m_treeWidget->show();
          this->sleep(1);

          index = findSimPortByPortName(simPort);//index of SIMs;
          delaySeconds(1);
          PortSIMList.at(index)->setSimCardStatus(READY);


       }
       else if(st==DialFailed)
       {
           stepsInfoOneNum.teleStep = DIALFAILED;
           m_teleProStepList.insert(index,stepsInfoOneNum);
           m_treeWidget->topLevelItem(index)->setText(1,"拨号失败");
           m_treeWidget->show();
           this->sleep(1);

           index = findSimPortByPortName(simPort);//index of SIMs;
           PortSIMList.at(index)->setSimCardStatus(READY);

       }

       mutex.lock();
       proInfoListFromSIMs.removeAt(0);
       mutex.unlock();
    }
    else
    {
       delayMilliSeconds(2000);
       emit needInteract();
    }

}

void CModemPool::run()
{
     while(!stopped)
     {
         processStatusChange();
     }
     stopped = false;
}

void CModemPool::stop()
{
    stopped = true;
}

int CModemPool::findSimPortByPortName(QString portName)
{
    int rev = -1;
    int num = portsCount();
    int i=0;
    for(i=0;i<num;i++)
    {
        if(portsInfo.at(i).portName()==portName)
            break;
    }

    if(i<num)
    {
        rev = i;
    }
    else
    {
        rev = -1;
    }

    return rev;

}

int CModemPool::getNextIndexToProcess()
{
    int rev = -1;
    int num = m_teleProStepList.count();
    int i=0;
    for(i=0;i<num;i++)
    {
        if(m_teleProStepList.at(i).teleStep == NOT_PROCESSED)
            break;
    }
    if(i<num)
    {
       rev = i;
    }
    else
    {
        rev = -1;
    }
    return rev;
}

void CModemPool::preparePorts()
{
    int num = portsInfo.count();
    int i=0;
    qDebug()<<"number of ports "<<num;

    /*send AT+COPS?\n to all ports*/
    for(i=0;i<num;i++)
    {
        SIM_status st = PortSIMList.at(i)->getSimStatus();
        qDebug()<<"sim"<<i<<" status:"<<st;
        if(st==IDLE)
        {
            PortSIMList.at(i)->write("AT+COPS?\n");
        }
    }

}

void CModemPool::interact()
{
    int num = portsInfo.count();
    int i=0;
   // qDebug()<<"number of ports "<<num;

    /*send AT+COPS?\n to all ports*/
    this->sleep(1);
    for(i=0;i<num;i++)
    {
        SIM_status st = PortSIMList.at(i)->getSimStatus();
        qDebug()<<"sim"<<i<<" status:"<<st;

        switch (st) {
        case IDLE:
           // PortSIMList.at(i)->write("AT+COPS?\n");
            break;
        case READY:
        {
            int indexNumToProcess = getNextIndexToProcess();
            if(indexNumToProcess!=-1)
            {
                QString strToSend = numsNeedProcess.at(indexNumToProcess);
                //delayMilliSeconds(1000);
                strToSend = "ATD"+strToSend+";\n";
                QByteArray ba = strToSend.toLatin1();
                char* ch = ba.data();
                PortSIMList.at(i)->write(ch);

            }
            else
            {
                this->stop();
                numClicked = 2;
                //setting the text of btn to be "OK"
                m_pBtn->setText("确定");
            }
        }
            break;
        case DialingOut:
            PortSIMList.at(i)->write("AT+CLCC\n");
            break;
        case WaitForFeedBack:
            PortSIMList.at(i)->write("ATH\n");
            break;
        case DialFailed:
            PortSIMList.at(i)->write("ATH\n");
            break;
        case SimInserted:
            PortSIMList.at(i)->write("AT+CLIP=1\n");
            break;
        case NeedRegist:
            PortSIMList.at(i)->write("AT+COPS?\n");
            break;

        default:
            break;
        }
    }
}

void CModemPool::setPushButton(QPushButton * pBtnToSet)
{
    m_pBtn = pBtnToSet;
}

void CModemPool::setTreeWidget(QTreeWidget *treeToSet)
{
    m_treeWidget = treeToSet;
}

int CModemPool::getNumClicked()
{
   return numClicked;
}

void CModemPool::setCommRecordTable(CCommRecordTable * CommRecordTable)
{
    m_CommRecordTable = CommRecordTable;
}
