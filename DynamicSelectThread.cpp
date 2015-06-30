#include "DynamicSelectThread.h"
#include <QDebug>

extern QMutex mutex;


QStringList KeyWordToSelect;

CDynamicSelectThread::CDynamicSelectThread(MainWindow *mainwindow)
{
    stopped = false;
    m_ContactorTable = new CContactorsTable();
    m_CommRecordTable = new CCommRecordTable();
    m_mainwindow = mainwindow;
    KeyWordToSelect.clear();
}

void CDynamicSelectThread::run()
{
     while(!stopped)
     {
         SelectRecords();
         this->sleep(2);
     }
     stopped = false;
}


void CDynamicSelectThread::stop()
{
    stopped = true;
}

void CDynamicSelectThread::SelectRecords()
{
    QStringList KeyWordList;
    QString KeyWord;
    mutex.lock();
    KeyWordList=KeyWordToSelect;
    mutex.unlock();
    int n = KeyWordList.count();
    int i=0;
    if(n!=0)
    {
        KeyWord = KeyWordList.takeAt(n-1);
        int index = KeyWord.left(1).toInt();
        KeyWord = KeyWord.right(KeyWord.length()-1);
        for(i=0;i<KeyWord.length();i++)
        {
            QChar oneChar = KeyWord.at(i);
            oneChar = oneChar.toLower();
            KeyWord.replace(i,1,oneChar);
        }
        if((index==0)||(index==2))
        {
            /*get contactors only here*/
            QString strSql = "select * from contactors where name like \'%"
                             + KeyWord + "%\' or pinyin like \'%"
                             + KeyWord + "%\' or ShortPinyin like \'%"
                             + KeyWord + "%\' or telenumber like \'%"
                             + KeyWord + "%\'";

            qDebug()<<"CDynamicSelectThread::SelectRecords  " + strSql;
            ContactorInfoList tempContactorList = m_ContactorTable->getListBySql(strSql);
            qDebug()<<"select result number  " + QString::number(tempContactorList.count());
            m_mainwindow->setFreshList(tempContactorList);
            m_mainwindow->RefreshContent(index,0);
        }
        else if(index==1)
        {
            /*get the top communication list and corresponding contactor list*/
            QString strSql = "select * from contactors where name like \'%"
                             + KeyWord + "%\' or pinyin like \'%"
                             + KeyWord + "%\' or ShortPinyin like \'%"
                             + KeyWord + "%\'";
            ContactorInfoList tempContactorList = m_ContactorTable->getListBySql(strSql);
            m_mainwindow->setFreshList(tempContactorList);

            strSql = "select max(startTime) as m_startTime,telenumber from communicate_record ";
            strSql += "where telenumber like \'%" + KeyWord + "%\'";
            strSql += "group by telenumber ";
            strSql += "order by max(startTime) DESC";

            CommRecordTopList tempTopCommRecordList = m_CommRecordTable->getListTopBySql(strSql);
            qDebug()<<"comm record table select result " + QString::number(tempTopCommRecordList.count());
            m_mainwindow->setFreshList(tempTopCommRecordList);
            m_mainwindow->RefreshContent(1,0);

        }

        for(i=0;i<n;i++)
        {
            mutex.lock();
            KeyWordToSelect.removeAt(0);
            mutex.unlock();
        }
    }
}
