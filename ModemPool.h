#ifndef MODEMPOOL_H
#define MODEMPOOL_H

#include "ModemPoolSerialPort.h"
#include <QThread>
#include <QMutex>
#include <QTreeWidget>
#include <QPushButton>
#include "CommRecordTable.h"

class MainWindow;
class CCommRecordTable;
extern QStringList numsNeedProcess;

typedef enum
{
    NOT_PROCESSED,
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
}teleProSteps;


typedef QList<teleProSteps> teleProStepList;

extern processInfoList proInfoListFromSIMs;

class CModemPool:public QThread
{
    Q_OBJECT
private: CModemPool();
    static CModemPool * _instance;
    serialPortInfoList portsInfo;
    QList<CModemPoolSerialPort *> PortSIMList;
    QStringList teleNumsProcessing;
    processInfoList m_proInfoList;//all sim card;
    teleProStepList m_teleProStepList;//all numbers to process
    volatile bool stopped;
    QPushButton * m_pBtn;
    QTreeWidget * m_treeWidget;
    CCommRecordTable * m_CommRecordTable;
    int numClicked;
public:
    static CModemPool * getInstance();
    CModemPoolSerialPort * getSIMPort(int index);
    QSerialPortInfo getPortInfo(int index);
    int findSimPortByPortName(QString portName);
    int getNextIndexToProcess();
    bool isAllProcessed();

    void setPushButton(QPushButton * pBtnToSet);
    void setTreeWidget(QTreeWidget *treeToSet);
    void setCommRecordTable(CCommRecordTable * CommRecordTable);

    int getNumClicked();

    void scanPorts();
    int portsCount();
    void closeAllPorts();
    //for thread
    void stop();
signals:
    void needInteract();
    void endProcess();
private slots:
    void startProcess();
    void preparePorts();
    void interact();
protected:
    void run();
    void processStatusChange();

};


#endif // MODEMPOOL_H

