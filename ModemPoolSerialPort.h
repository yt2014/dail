#ifndef MODEMPOOLSERIALPOOL
#define MODEMPOOLSERIALPOOL

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QMutex>


void delaySeconds(int n);
void delayMilliSeconds(int n);

typedef enum
{
    IDLE,
    READY,
    DialingOut,
    MessageReceived,
    ComeRing

}SIM_status;

typedef QList<QSerialPortInfo> serialPortInfoList;


class CModemPoolSerialPort:public QSerialPort,public QThread
{
    Q_OBJECT
   public: CModemPoolSerialPort();
           ~CModemPoolSerialPort();
           void setSimCardStatus(SIM_status status);
           void stop();

   public:           
       void close();

   protected:
       void run();
       void processData();

   private:
       volatile bool stopped;
       QStringList dataReceived;
       SIM_status simCardStatus;

  private slots:
       void receiveData();
};




#endif
