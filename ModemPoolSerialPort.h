#ifndef MODEMPOOLSERIALPOOL
#define MODEMPOOLSERIALPOOL

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>


class CModemPoolSerialPort:QSerialPort
{
   private: CModemPoolSerialPort();

           static CModemPoolSerialPort * _Instance;

   public:
       static CModemPoolSerialPort * getInstance();

};




#endif
