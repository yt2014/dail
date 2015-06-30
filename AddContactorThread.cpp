#include "AddContactorThread.h"
#include <QDebug>
#include <QMutex>

QMutex mutex;

ContactorInfoList ContactorlistToAdd;

AddContactorThread::AddContactorThread()
{
    stopped = false;
    m_ContactorTable = new CContactorsTable();
 }

void AddContactorThread::run()
{
     while(!stopped)
     {
         AddToContactorTable();
         sleep(2);
     }
     stopped = false;
 }

 void AddContactorThread::stop()
 {
     stopped = true;
 }

 void AddContactorThread::setMessage(QString message)
 {
     messageStr = message;
 }

 void AddContactorThread::AddToContactorTable()
 {
     ContactorInfoList tempList;
     mutex.lock();
     tempList = ContactorlistToAdd;
     mutex.unlock();

     ContactorInfo infoToAdd;
     if(tempList.count()!=0)
     {
         infoToAdd = tempList.takeAt(0);
         m_ContactorTable->InsertPinyinForRecord(infoToAdd);
         mutex.lock();
         ContactorlistToAdd.removeAt(0);
         mutex.unlock();
     }

 }
