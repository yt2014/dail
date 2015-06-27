﻿#ifndef MODEMPOOL_H
#define MODEMPOOL_H

#include "ModemPoolSerialPort.h"

extern QStringList numsNeedProcess;

class CModemPool
{
private: CModemPool();
    static CModemPool * _instance;
    serialPortInfoList portsInfo;
    QList<CModemPoolSerialPort *> PortSIMList;
    QStringList teleNumsProcessing;
public:
    static CModemPool * getInstance();
    CModemPoolSerialPort * getSIMPort(int index);
    QSerialPortInfo getPortInfo(int index);

    void scanPorts();
    int portsCount();
    void closeAllPorts();

};


#endif // MODEMPOOL_H

