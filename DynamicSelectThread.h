#ifndef DYNAMICSELECTTHREAD_H
#define DYNAMICSELECTTHREAD_H

#include <QThread>
#include <QMutex>
#include "CContatorsTable.h"
#include "CommRecordTable.h"
#include "mainwindow.h"

class MainWindow;
extern QStringList KeyWordToSelect;

class CDynamicSelectThread : public QThread
{
     Q_OBJECT
 public:
     CDynamicSelectThread(MainWindow *mainwindow);
     void stop();

 protected:
     void run();
     void SelectRecords();

 private:
     volatile bool stopped;
     CContactorsTable* m_ContactorTable;
     CCommRecordTable* m_CommRecordTable;

     MainWindow * m_mainwindow;
 };





#endif // DYNAMICSELECTTHREAD_H
