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
    isAllProcessed = true;
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
        if(len!=0)
        {
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
        processInfo proInfoToInit;
        for(i=0;i<num;i++)
        {
           SIM_status st = PortSIMList.at(i)->getSimStatus();
           proInfoToInit.simPort = PortSIMList.at(i)->portName();
           proInfoToInit.processStatus = st;

           qDebug()<<"start pro sim"<<i<<" status:"<<st;
           if(i<len)
           {
             proInfoToInit.telenumber = numsNeedProcess.at(i);
             m_proInfoList.append(proInfoToInit);
             if(st==READY)
             {
                 strDial = "ATD"+numsNeedProcess.at(i) + ";\n";
                 QByteArray ba = strDial.toLatin1();

                 char* ch = ba.data();
                 PortSIMList.at(i)->write(ch);

             }
             else if(st==IDLE)
             {
                  PortSIMList.at(i)->write("AT+COPS?\n");
             }
           }

       }
        //qDebug()<<"thread is running? "<<this->isRunning();
       // this->start();
       // this->run();

       numClicked = 1;
       //setting the text of btn to be "stop"
       //m_mainWindow->ui
       m_pBtn->setText("停止");
       isAllProcessed = false;
        }
    }
    else if(numClicked==1)
    {

        numClicked = 2;
        //setting the text to be "ok"
        m_pBtn->setText("确定");

        int numOfAllSims =  PortSIMList.count();
        int numOfSimsInPro = m_proInfoList.count();
        int i = 0;
        QByteArray ba;
        char * ca;
        for(i=0;i<numOfAllSims;i++)
        {
            if(i<numOfSimsInPro)
            {
                if(m_proInfoList.at(i).processStatus>READY)
                {
                    ba = "ATH\n";
                    ca = ba.data();
                    PortSIMList.at(i)->write(ca);
                }
                else
                {
                    ba = "AT+COPS?\n";
                    ca = ba.data();
                    PortSIMList.at(i)->write(ca);
                }
            }
            else
            {
                ba = "AT+COPS?\n";
                ca = ba.data();
                PortSIMList.at(i)->write(ca);
            }
        }
        isAllProcessed = true;
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
       isAllProcessed = true;
    }


}

void CModemPool::processStatusChange()
{

   // qDebug()<<"m_modem thread is running";
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

       int index = -1;//index of tele number, search in tele number list

       int indexSim = findSimPortByPortName(simPort);


       qDebug()<<"index sim "<<indexSim<<"all processed?"<<isAllProcessed;
       infoDecoded.indexOfSim = indexSim;       
       infoDecoded.simST = st;
       teleProSteps stepsInfoOneNum;
       if(!isAllProcessed)
       {
           SIM_status tempStatus = m_proInfoList.at(indexSim).processStatus;
           if((st>READY)||((st==READY)&&(tempStatus>READY)))
           {

                  if(telenumber!="")
                  {
                      index = numsNeedProcess.indexOf(telenumber);
                  }
                  else
                  {
                     if(indexSim<m_proInfoList.count())
                     {
                         qDebug()<<"num of m_proInfoList "<<m_proInfoList.count();

                         telenumber = m_proInfoList.at(indexSim).telenumber;
                         index = numsNeedProcess.indexOf(telenumber);
                      }
                      else
                      {
                         index = -1;
                      }
                  }

           }


           if(st==DialingOut)
           {

               qDebug()<<"dailing out received, index of tele "<<index<<" "<<telenumber;
               stepsInfoOneNum = m_teleProStepList.takeAt(index);
               stepsInfoOneNum.teleStep = DAILING_OUT;
               stepsInfoOneNum.recordToStore.startTime = QDateTime::currentDateTime();
               stepsInfoOneNum.recordToStore.isCallIn = 0;
               m_teleProStepList.insert(index,stepsInfoOneNum);
               m_treeWidget->topLevelItem(index)->setText(1,"拨号中。。。");
               m_treeWidget->show();
               this->sleep(1);
               //emit needInteract();
           }
           else if(st == READY)
             {
                 qDebug()<<"to ready from "<<tempStatus<<"index of tele "<<index<<"num "<<telenumber;
                 if(tempStatus == WaitForFeedBack)
                 {
                     //qDebug()<<"from waitforfeedback to READY";

                    stepsInfoOneNum = m_teleProStepList.takeAt(index);
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
                    //emit needInteract();
                    // PortSIMList.at(indexSim)->setSimCardStatus(READY);

                 }
                 else if(tempStatus==DialFailed)
                 {
                     //qDebug()<<"from DialFailed to READY";
                     stepsInfoOneNum = m_teleProStepList.takeAt(index);
                     stepsInfoOneNum.teleStep = DIALFAILED;
                     m_teleProStepList.insert(index,stepsInfoOneNum);
                     m_treeWidget->topLevelItem(index)->setText(1,"拨号失败");
                     m_treeWidget->show();
                     this->sleep(1);

                     //emit needInteract();
                     //index = findSimPortByPortName(simPort);//index of SIMs;
                    //PortSIMList.at(index)->setSimCardStatus(READY);
                 }
                 else if(tempStatus>READY)
                 {
                     stepsInfoOneNum = m_teleProStepList.takeAt(index);
                     stepsInfoOneNum.teleStep = PROCESS_FINISHED;
                     m_teleProStepList.insert(index,stepsInfoOneNum);
                     m_treeWidget->topLevelItem(index)->setText(1,"本地中断");
                     m_treeWidget->show();
                     this->sleep(1);
                 }

             }
           infoDecoded.indexOfTel = index;
       }
       emit needInteract();
       sleep(1);
       mutex.lock();
       proInfoListFromSIMs.removeAt(0);
       mutex.unlock();
       sleep(5);
    }
    else
    {
        if(!isAllProcessed)
        {
           int numOfProcessing = m_proInfoList.count();
           //int numOfAllSims = PortSIMList.count();
           SIM_status tempStatus;
           for (int i=0;i<numOfProcessing;i++)
           {
               tempStatus = m_proInfoList.at(i).processStatus;
               if(tempStatus==DialingOut)
               {
                   infoDecoded.indexOfSim = i;
                   infoDecoded.simST = DialingOut;
                   emit needInteract();
                   sleep(1);
               }
           }
        }
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
   // qDebug()<<"m_modem thread stopped";
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

bool CModemPool::checkAllProcessed()
{
    bool rev = false;
    int num = m_teleProStepList.count();
   // qDebug()<<"in is AllProcessed "<<num;
    int i=0;
    for(i=0;i<num;i++)
    {

        if( (m_teleProStepList.at(i).teleStep != PROCESS_FINISHED)
           &&(m_teleProStepList.at(i).teleStep != DIALFAILED)
           &&(m_teleProStepList.at(i).teleStep != SEND_MESSAGE_FAIL))
            break;
        //if find a number with status that not ended
    }
    if(i<num)
    {
       rev = false;
    }
    else
    {
        rev = true;
    }
   // qDebug()<<"return from AllProcessed ";
    return rev;

}

void CModemPool::preparePorts()
{
    int num = portsInfo.count();
    int i=0;
    qDebug()<<"preparePorts number of ports "<<num;

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


    /*send AT+COPS?\n to all ports*/

        // qDebug()<<"number of ports in interact "<<num;
            SIM_status stNew = infoDecoded.simST;
            int index_Sim = infoDecoded.indexOfSim;
            //SIM_status stLast = m_proInfoList.at(indexSim).processStatus;
            qDebug()<<"in interact sim"<<index_Sim<<" status:"<<stNew;

            switch (stNew) {
            case IDLE:
               PortSIMList.at(index_Sim)->write("AT+COPS?\n");
            break;
            case READY:
            {
                if(!isAllProcessed)
                {
                int indexNumToProcess = getNextIndexToProcess();

                if(indexNumToProcess!=-1)
                {
                     teleProSteps stepsInfoOneNum = m_teleProStepList.takeAt(indexNumToProcess);
                     stepsInfoOneNum.teleStep = START_PROCESS;
                     m_teleProStepList.insert(indexNumToProcess,stepsInfoOneNum);
                     QString strToSend = numsNeedProcess.at(indexNumToProcess);
                     processInfo proInfoToInit;

                     if(index_Sim<m_proInfoList.count())
                     {
                       proInfoToInit = m_proInfoList.takeAt(index_Sim);
                       proInfoToInit.telenumber = strToSend;
                       proInfoToInit.processStatus = stNew;
                       m_proInfoList.insert(index_Sim,proInfoToInit);

                     }
                     else
                     {
                       proInfoToInit.simPort = PortSIMList.at(index_Sim)->portName();
                       proInfoToInit.telenumber = strToSend;
                       proInfoToInit.processStatus = stNew;
                       m_proInfoList.append(proInfoToInit);
                     }



                     //delayMilliSeconds(1000);
                     strToSend = "ATD"+strToSend+";\n";
                     QByteArray ba = strToSend.toLatin1();
                     char* ch = ba.data();
                     PortSIMList.at(index_Sim)->write(ch);

                }
                else
                {
                     qDebug()<<"no tele to dail again";
                     isAllProcessed = checkAllProcessed();
                     if(isAllProcessed)
                     {

                         numClicked = 2;
                         //setting the text of btn to be "OK"
                         m_pBtn->setText("确定");
                     }

                }
                }
            }
            break;
            case DialingOut:
                PortSIMList.at(index_Sim)->write("AT+CLCC\n");
                break;
            case WaitForFeedBack:
                //qDebug()<<"send ATH";
                PortSIMList.at(index_Sim)->write("ATH\n");
                break;
            case DialFailed:
                PortSIMList.at(index_Sim)->write("ATH\n");
                break;
            case SimInserted:
                PortSIMList.at(index_Sim)->write("AT+CLIP=1\n");
                break;
            case NeedRegist:
                PortSIMList.at(index_Sim)->write("AT+COPS?\n");
                break;

           default:
               break;
           }
            if((!isAllProcessed)&&(index_Sim<m_proInfoList.count()))
            {
               processInfo proInfoToInit = m_proInfoList.takeAt(index_Sim);
               proInfoToInit.processStatus = stNew;
               m_proInfoList.insert(index_Sim,proInfoToInit);
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
