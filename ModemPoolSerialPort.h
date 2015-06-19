#ifndef MODEMPOOLSERIALPOOL
#define MODEMPOOLSERIALPOOL

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>



void delaySeconds(int n);
void delayMilliSeconds(int n);



typedef QList<QSerialPort *> serialPortList;

extern serialPortList ports;

class CModemPoolSerialPort:public QSerialPort
{
   private: CModemPoolSerialPort();

           static CModemPoolSerialPort * _Instance;


   public:
       static CModemPoolSerialPort * getInstance();
       void close();

};




#endif
