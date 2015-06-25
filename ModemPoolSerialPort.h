#ifndef MODEMPOOLSERIALPOOL
#define MODEMPOOLSERIALPOOL

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>



void delaySeconds(int n);
void delayMilliSeconds(int n);



typedef QList<QSerialPortInfo> serialPortInfoList;

extern serialPortInfoList portsInfo;

class CModemPoolSerialPort:public QSerialPort
{
   public: CModemPoolSerialPort();
           ~CModemPoolSerialPort();

   public:           
       void close();
};




#endif
