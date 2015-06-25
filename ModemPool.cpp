#include "ModemPool.h"
#include <QDebug>

CModemPool * CModemPool::_instance = NULL;

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

