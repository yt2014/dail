#include "ModemPoolSerialPort.h"
#include <QDebug>

CModemPoolSerialPort * CModemPoolSerialPort::_Instance = NULL;


CModemPoolSerialPort::CModemPoolSerialPort()
{
   // this->setBaudRate();
   // this->setDataBits();

    qDebug()<<"databits is "+QString::number(this->dataBits());
    qDebug()<<"baudRate is " + QString::number(this->baudRate());

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
           qDebug() << "Name        : " << info.portName();
           qDebug() << "Description : " << info.description();
           qDebug() << "Manufacturer: " << info.manufacturer();

           // Example use QSerialPort
           QSerialPort serial;
           serial.setPort(info);
           serial.setBaudRate(QSerialPort::Baud115200);
           if (serial.open(QIODevice::ReadWrite))
           {
               qDebug()<<"open com port successfully";
           }

       }

}

CModemPoolSerialPort *CModemPoolSerialPort::getInstance()
{
    if(_Instance==NULL)
    {
        _Instance = new CModemPoolSerialPort();
    }

    return _Instance;
}
