#ifndef MODEMPOOLSERIALPOOL
#define MODEMPOOLSERIALPOOL

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QMutex>

class CSerialPortThread;


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


class CModemPoolSerialPort:public QSerialPort
{
    Q_OBJECT
   public: CModemPoolSerialPort();
           ~CModemPoolSerialPort();
           void setSimCardStatus(SIM_status status);
           SIM_status getSimStatus();
   public:
       void close(); 
       void processData();

   private:       
       QStringList dataReceived;
       SIM_status simCardStatus;
       CSerialPortThread * m_threadForSim;

   private slots:
       void receiveData();

};

class CSerialPortThread:public QThread
{
    Q_OBJECT
public:
    CSerialPortThread(CModemPoolSerialPort*SimCardPort);
    void stop();
protected:
    void run();
private:
    volatile bool stopped;
    CModemPoolSerialPort * m_simCardPort;

};




#endif
