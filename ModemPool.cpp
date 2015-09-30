#include "ModemPool.h"
#include <QDebug>
#include <QFile>

CModemPool * CModemPool::_instance = NULL;

QStringList numsNeedProcess;
processInfoList proInfoListFromSIMs; //this global list is used to synchronize the thread.
extern QMutex mutex;

extern QTextStream logfile;
extern QFile file;
QFile * logFile;

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
    m_proType = Dial;
    m_ShortMessageTable = NULL;
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
    if(m_proType == Dial)
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
           int indexTeleToPro = getNextIndexToProcess();
           if(indexTeleToPro!=-1)
           {
             proInfoToInit.telenumber = m_teleProStepList.at(indexTeleToPro).telenumber;

             if(st==READY)
             {
                 strDial = "ATD"+numsNeedProcess.at(indexTeleToPro) + ";\n";//\n
                 //strDial = "ATD13541137539;\n";
                 QByteArray ba = strDial.toLatin1();

                 char* ch = ba.data();
                 PortSIMList.at(i)->write(ch);

                // logFile->write("dial now\n");

                 m_proInfoList.append(proInfoToInit);
             }
             else //if(st==IDLE)
             {
                  //PortSIMList.at(i)->write("AT+COPS?\n");
                  infoDecoded.indexOfSim = i;
                  infoDecoded.simST = st;
                  //emit needInteract();
                  //delayMilliSeconds(100);
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
        emit startProcessToMainUI();
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
        logFile->write("set to 1 --1\n");
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
       logFile->write("set to 1---2\n");
    }
    }
    else if(m_proType==Message)
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
                  oneTeleProStepInfo.MsgRecordToStore.NumberRemote = oneTeleProStepInfo.telenumber;

                  m_teleProStepList.append(oneTeleProStepInfo);
             }

             /*为所有可用的SIM卡分配需处理的电话号码，并且开始处理（拨号/发信息）*/
             QString strDial;
             proInfoListFromSIMs.clear();
             processInfo proInfoToInit;
             file.setFileName("log.txt");
             logfile.setDevice(&file);
             for(i=0;i<num;i++)//num is the number of ports;
             {
                SIM_status st = PortSIMList.at(i)->getSimStatus();
                proInfoToInit.simPort = PortSIMList.at(i)->portName();
                proInfoToInit.processStatus = st;
                teleProSteps oneTeleProStepInfoToChange;
                qDebug()<<"start pro sim"<<i<<" status:"<<st;
                logfile<<"start pro sim"<<i<<" status:"<<st;
                QString strlog = "start pro sim" + QString::number(i) + " status:" + QString::number(st)+"\n";
                logFile->write(strlog.toLatin1());
                int indexTeleToPro = getNextIndexToProcess();
                if(indexTeleToPro!=-1)
                {
                  proInfoToInit.telenumber = m_teleProStepList.at(indexTeleToPro).telenumber;
                  //QString strToWrite = "indexGot is " + QString::number(indexTeleToPro) + "\n";
                  //logFile->write(strToWrite.toLatin1());
                  if(st==ReadyForSendMessage)
                  {
                      strDial = "AT+CMGS=\""+CShortMessageTable::stringToUCS4String(numsNeedProcess.at(indexTeleToPro)) + "\"\n";

                    //  strDial = "AT+CMGS=\""+CShortMessageTable::stringToUCS4String("10086") + "\"\n";
                      QByteArray ba = strDial.toLatin1();

                      logfile<<"start send message " << strDial<<"\n";

                      logFile->write(ba);


                      char* ch = ba.data();

                      PortSIMList.at(i)->write(ch);
                     // PortSIMList.at(i)->write();

                      oneTeleProStepInfoToChange = m_teleProStepList.takeAt(indexTeleToPro);
                      oneTeleProStepInfoToChange.teleStep = START_PROCESS;
                      m_teleProStepList.insert(indexTeleToPro,oneTeleProStepInfoToChange);

                      m_proInfoList.append(proInfoToInit);
/**/
                      //logFile->write(strDial.toLatin1());
                      logFile->write(" send message here\n");
                  }
                  else //if(st==IDLE)
                  {
                       //PortSIMList.at(i)->write("AT+CMGF=1\n");
                       infoDecoded.indexOfSim = i;
                       infoDecoded.simST = st;
                       //isAllProcessed = false;

                       emit needInteract();
                       //delayMilliSeconds(100);
                       //logfile<<"send AT+CMGF=1\n";
                       //logFile->write("to process in interact\n");
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
            QString strlog = "is all processed?" + QString::number(isAllProcessed)+"\n";
            logFile->write(strlog.toLatin1());
             }
             emit startProcessToMainUI();
        }
        else if(numClicked==1)
        {
            numClicked = 2;
            //setting the text to be "ok"
            m_pBtn->setText("确定");

            int numOfAllSims =  PortSIMList.count();
            //int numOfSimsInPro = m_proInfoList.count();
            int i = 0;
            QByteArray ba;
            char * ca;
            for(i=0;i<numOfAllSims;i++)
            {

                    ba = "AT+CMGF=1\n";
                    ca = ba.data();
                    PortSIMList.at(i)->write(ca);
            }
            isAllProcessed = true;
            logFile->write("set to 1---3\n");
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
            logFile->write("set to 1---4\n");
        }
    }

}

void CModemPool::processStatusChange()
{

    //qDebug()<<"m_modem thread is running";
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

      // isAllProcessed = checkAllProcessed();
       QString logToWrite = "sim "+ QString::number(indexSim) + "status " + QString::number(st)+ " is all processed?"+QString::number(isAllProcessed)+"\n";

       logFile->write(logToWrite.toLatin1());

       infoDecoded.indexOfSim = indexSim;       
       infoDecoded.simST = st;
       teleProSteps stepsInfoOneNum;
       if(!isAllProcessed)
       {
           int indexSimInPro = findSimPortInProByPortName(simPort);
           SIM_status tempStatus = m_proInfoList.at(indexSimInPro).processStatus;
           if((st>READY)||((st==READY)&&(tempStatus>READY)))
           {

                  if(telenumber!="")
                  {
                      index = numsNeedProcess.indexOf(telenumber);
                  }
                  else
                  {
                     //if(indexSim<m_proInfoList.count())

                         qDebug()<<"num of m_proInfoList "<<m_proInfoList.count();

                         telenumber = m_proInfoList.at(indexSimInPro).telenumber;
                         index = numsNeedProcess.indexOf(telenumber);



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
           else if(st == ReadyForSendMessage)
           {
               qDebug()<<"to ready from "<<tempStatus<<"index of tele "<<index<<"num "<<telenumber;
               QString strlog = "to ready from "+ QString::number(tempStatus) + " "+ telenumber + "\n";
               logFile->write(strlog.toLatin1());
               if(tempStatus == NeedSendContext)
               {
                   stepsInfoOneNum = m_teleProStepList.takeAt(index);
                   stepsInfoOneNum.teleStep = PROCESS_FINISHED;
                   stepsInfoOneNum.MsgRecordToStore.isReceived = false;
                   stepsInfoOneNum.MsgRecordToStore.messageContext = m_msgToSend;
                   stepsInfoOneNum.MsgRecordToStore.NumberRemote = stepsInfoOneNum.telenumber;
                   stepsInfoOneNum.MsgRecordToStore.send_recvTime = QDateTime::currentDateTime();
                   stepsInfoOneNum.MsgRecordToStore.subID = 0;
                   m_teleProStepList.insert(index,stepsInfoOneNum);
                   if(m_ShortMessageTable!=NULL)
                   {
                       m_ShortMessageTable->addOneRecord(stepsInfoOneNum.MsgRecordToStore);
                   }
                   else
                   {
                       qDebug()<<"shorMessageTable is NULL";
                   }
                   m_treeWidget->topLevelItem(index)->setText(1,"发送完成");
                   m_treeWidget->show();
                   logFile->write("send text message finished\n");
                   this->sleep(1);

               }
           }
           else if(st == NeedSendContext)
           {

               m_treeWidget->topLevelItem(index)->setText(1,"发送中。。。");
               m_treeWidget->show();
               QString strlog = "sending index of tele is "+QString::number(index)+ "\n";
               logFile->write(strlog.toLatin1());
           }
           infoDecoded.indexOfTel = index;
       }
       emit needInteract();
      // sleep(1);
       mutex.lock();
       proInfoListFromSIMs.removeAt(0);
       mutex.unlock();
      // sleep(5);
      // logFile->close();
      // logFile->open(QIODevice::WriteOnly|QIODevice::Text);
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
               QString portname = m_proInfoList.at(i).simPort;
               int indexSim = findSimPortByPortName(portname);
               if(tempStatus==DialingOut)
               {
                   infoDecoded.indexOfSim = indexSim;
                   infoDecoded.simST = DialingOut;
                   emit needInteract();
                   sleep(4);
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

int CModemPool::findSimPortInProByPortName(QString portName)
{
    int rev = -1;
    int num = m_proInfoList.count();
    int i=0;
    for(i=0;i<num;i++)
    {
        if(m_proInfoList.at(i).simPort == portName)
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
    logFile->write("in AllProcessed");
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

            PortSIMList.at(i)->write("AT+COPS?\n");


    }
    setProType(Dial);

}

 void CModemPool::preparePortsForSendingMsg()
 {
     int num = portsInfo.count();
     int i=0;
     qDebug()<<"preparePorts number of ports "<<num;

     /*send AT+CMGF=1\n to all ports*/
     for(i=0;i<num;i++)
     {
         SIM_status st = PortSIMList.at(i)->getSimStatus();
         qDebug()<<"in pre msg sim"<<i<<" status:"<<st;
         if((st==IDLE)||(st==READY))
         {
             PortSIMList.at(i)->write("AT+CMGF=1\n");
         }
     }
     setProType(Message);
 }

void CModemPool::interact()
{


    /*send AT+COPS?\n to all ports*/

        // qDebug()<<"number of ports in interact "<<num;
            SIM_status stNew = infoDecoded.simST;
            int index_Sim = infoDecoded.indexOfSim;
            //SIM_status stLast = m_proInfoList.at(indexSim).processStatus;
            qDebug()<<"in interact sim"<<index_Sim<<" status:"<<stNew;
            QString strLog = "in interact sim" + QString::number(index_Sim)+" status:"+QString::number(stNew)+"\n";
            logFile->write(strLog.toLatin1());
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

                     /*if(index_Sim<m_proInfoList.count())
                     {
                       proInfoToInit = m_proInfoList.takeAt(index_Sim);
                       proInfoToInit.telenumber = strToSend;
                       proInfoToInit.processStatus = stNew;
                       m_proInfoList.insert(index_Sim,proInfoToInit);

                     }
                     else
                     {*/
                       proInfoToInit.simPort = PortSIMList.at(index_Sim)->portName();
                       proInfoToInit.telenumber = strToSend;
                       proInfoToInit.processStatus = stNew;
                       m_proInfoList.append(proInfoToInit);




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
                delayMilliSeconds(50);
                PortSIMList.at(index_Sim)->write("AT+CMGF=1\n");
                break;
            case ReadyForSendMessage:
            {
                qDebug()<<"ready for send message";
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

                   /*  if(index_Sim<m_proInfoList.count())
                     {
                       proInfoToInit = m_proInfoList.takeAt(index_Sim);
                       proInfoToInit.telenumber = strToSend;
                       proInfoToInit.processStatus = stNew;
                       m_proInfoList.insert(index_Sim,proInfoToInit);

                     }
                     else
                     {*/
                       proInfoToInit.simPort = PortSIMList.at(index_Sim)->portName();
                       proInfoToInit.telenumber = strToSend;
                       proInfoToInit.processStatus = stNew;
                       m_proInfoList.append(proInfoToInit);




                     //delayMilliSeconds(1000);
                     strToSend = "AT+CMGS=\""+CShortMessageTable::stringToUCS4String(strToSend)+"\"\n";
                     QByteArray ba = strToSend.toLatin1();
                     char* ch = ba.data();
                     logFile->write(ba);
                     PortSIMList.at(index_Sim)->write(ch);
                }
                else
                {
                     qDebug()<<"no tele to send message again";
                     logFile->write("no tele to send message again\n");
                     isAllProcessed = checkAllProcessed();
                     if(isAllProcessed)
                     {

                         numClicked = 2;
                         //setting the text of btn to be "OK"
                         m_pBtn->setText("确定");
                     }

                }
                }
                QString strLog1 = "isAllProcessed:" + QString::number(isAllProcessed)+"\n";
                logFile->write(strLog1.toLatin1());
            }
                //PortSIMList.at(index_Sim)->write("AT+CMGS=\n");
                break;
            case NeedSendContext:
            {
                QString strFromText = m_msgToSend;
                QString strToSend = CShortMessageTable::stringToUCS4String(strFromText)+"\x1a\n";
                qDebug()<<"str to send " + strToSend;
                QByteArray ba = strToSend.toLatin1();
                char* ch = ba.data();
                logFile->write("send text message now");
                logFile->write(ba);
                PortSIMList.at(index_Sim)->write(ch);
            }
                break;
             case SetForSendMsgStep1:
            {
                //delaySeconds(1);
                PortSIMList.at(index_Sim)->write("AT+CSCS=\"UCS2\"\n");
            }
                break;
            case SetForSendMsgStep2:
            {
                //delaySeconds(1);
                PortSIMList.at(index_Sim)->write("AT+CSMP=17,167,2,25\n");
            }
                break;
           default:
               break;
           }
            if((!isAllProcessed)/*&&(index_Sim<m_proInfoList.count())*/)
            {
               int indexOftele = findSimPortInProByPortName(PortSIMList.at(index_Sim)->portName());
               if(indexOftele!=-1)
               {
                  processInfo proInfoToInit = m_proInfoList.takeAt(indexOftele);
                  proInfoToInit.processStatus = stNew;
                  m_proInfoList.insert(indexOftele,proInfoToInit);
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

void CModemPool::setShortMsgTable(CShortMessageTable * shorMsgTable)
{
    m_ShortMessageTable = shorMsgTable;
}

void CModemPool::setProType(processType proType)
{
    m_proType = proType;
}

void CModemPool::setMsgToSend(QString str)
{
    m_msgToSend = str;
}
