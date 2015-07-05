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
    SimInserted,
    NeedRegist,
    SimNotAvailable,
    READY,
    DialingOut,
    WaitForCommandResult,
    WaitForFeedBack,
    MessageReceived,
    DialFailed,
    ComeRing,
    ReadyForReceiveMessage,
    ReadyForSendMessage,
    SendingMessage,
    MessageSendSuccess,
    MessageSendFailed,
    NeedSendContext

}SIM_status;


typedef struct
{
    QString simPort;
    SIM_status processStatus;
    QString telenumber;
}processInfo;

typedef QList<processInfo> processInfoList;

typedef QList<QSerialPortInfo> serialPortInfoList;

extern processInfoList proInfoListFromSIMs;
class CModemPoolSerialPort:public QSerialPort
{
    Q_OBJECT
   public: CModemPoolSerialPort();
           ~CModemPoolSerialPort();
           void setSimCardStatus(SIM_status status);
           SIM_status getSimStatus();
           QString m_telenumber;
   public:
       void close(); 
       void processData();

   private:       
       QStringList dataReceived;
       SIM_status simCardStatus;
       SIM_status recoverStatus;
       CSerialPortThread * m_threadForSim;
       int counterRecv;
       QString m_ucs4StrGet;
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
