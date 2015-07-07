#ifndef MODEMPOOL_H
#define MODEMPOOL_H

#include "ModemPoolSerialPort.h"
#include <QThread>
#include <QMutex>
#include <QTreeWidget>
#include <QPushButton>
#include "CommRecordTable.h"
#include "ShortMessageTable.h"

class MainWindow;
class CCommRecordTable;
extern QStringList numsNeedProcess;

typedef enum
{
    NOT_PROCESSED,
    START_PROCESS,
    SENDING_MESSAGE,
    DAILING_OUT,
    PROCESS_FINISHED,
    DIALFAILED,
    SEND_MESSAGE_FAIL
}processSteps;

typedef struct
{
    QString telenumber;
    processSteps teleStep;
    CommRecordInfo recordToStore;
    messageInfo MsgRecordToStore;
}teleProSteps;


typedef QList<teleProSteps> teleProStepList;

extern processInfoList proInfoListFromSIMs;//new status from SIM port.

typedef enum
{
    Message,
    Dial
}processType;

typedef struct
{
    int indexOfSim;
    int indexOfTel;
    SIM_status simST;
}decodedChangeInfo;

class CModemPool:public QThread
{
    Q_OBJECT
private: CModemPool();
    static CModemPool * _instance;
    serialPortInfoList portsInfo;
    QList<CModemPoolSerialPort *> PortSIMList;
    QStringList teleNumsProcessing;
    processInfoList m_proInfoList;//all sim card, store the last status;
    teleProStepList m_teleProStepList;//all numbers to process
    volatile bool stopped;
    QPushButton * m_pBtn;
    QTreeWidget * m_treeWidget;
    CCommRecordTable * m_CommRecordTable;
    CShortMessageTable * m_ShortMessageTable;
    int numClicked;
    bool isAllProcessed;
    decodedChangeInfo infoDecoded;
    processType m_proType;
    QString m_msgToSend;

public:
    static CModemPool * getInstance();
    CModemPoolSerialPort * getSIMPort(int index);
    QSerialPortInfo getPortInfo(int index);
    int findSimPortByPortName(QString portName);
    int getNextIndexToProcess();
    bool checkAllProcessed();

    void setPushButton(QPushButton * pBtnToSet);
    void setTreeWidget(QTreeWidget *treeToSet);
    void setCommRecordTable(CCommRecordTable * CommRecordTable);
    void setShortMsgTable(CShortMessageTable * shorMsgTable);

    void setProType(processType proType);
    void setMsgToSend(QString str);
    int getNumClicked();

    void scanPorts();
    int portsCount();
    void closeAllPorts();
    //for thread
    void stop();
    void preparePortsForSendingMsg();
signals:
    void needInteract();
    void endProcess();
    void startProcessToMainUI();
private slots:
    void startProcess();
    void preparePorts();
    void interact();

protected:
    void run();
    void processStatusChange();

};



#endif // MODEMPOOL_H

