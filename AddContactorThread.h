#ifndef ADDCONTACTORTHREAD_H
#define ADDCONTACTORTHREAD_H


#include <QThread>
#include <iostream>
#include "CContatorsTable.h"



class AddContactorThread : public QThread
{
     Q_OBJECT
 public:
     AddContactorThread();
     void setMessage(QString message);
     void stop();



 protected:
     void run();
     void AddToContactorTable();

 private:
     QString messageStr;
     volatile bool stopped;
     CContactorsTable* m_ContactorTable;
 };


#endif // ADDCONTACTORTHREAD_H
