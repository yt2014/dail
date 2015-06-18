#ifndef MODEMPOOLSERIALPOOL
#define MODEMPOOLSERIALPOOL

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>



void delaySeconds(int n);
void delayMilliSeconds(int n);

class CModemPoolSerialPort:public QSerialPort
{
   private: CModemPoolSerialPort();

           static CModemPoolSerialPort * _Instance;

   public:
       static CModemPoolSerialPort * getInstance();

};




#endif
