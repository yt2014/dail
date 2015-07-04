#ifndef SHORTMESSAGETABLE_H
#define SHORTMESSAGETABLE_H


#include "DatabaseTable.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDebug>
#include <QDateTime>

typedef struct
{
    QString NumberRemote;
    QString NumberLocal;
    QDateTime send_recvTime;
    bool isReceived;
    QString messageContext;
    int subID;
    bool readed;
}messageInfo;


typedef struct
{
    QString telenum;
    QDateTime send_recvTime;
}messageTopInfo;

typedef QList<messageInfo> messageInfoList;
typedef QList<messageTopInfo> messageTopInfoList;

typedef struct
{
  QString teleNumber;
  bool isNumberExist;
}remoteNumberInfo;

class CShortMessageTable:CDatabaseTable
{
public:
    CShortMessageTable(QString DatabaseAlias = "Dail",QString TableName = "shortMessage");
    virtual ~CShortMessageTable();
    void setDatabaseAlias(QString DatabaseAlias);
    void setTableName(QString TableName);
    messageInfoList getListAllFromDatabase();
    messageInfoList getListBySql(QString strSql);
    messageTopInfoList getListTop();
    messageTopInfoList getListTopBySql(QString strSql);
    int isRecordExist(messageInfo RecordToStore);//if not exist, return value set to -1, else return value set to the index.
    bool openDatabase();
    int isTeleNumExistInTopList(QString telenumber,messageTopInfoList topList);//if not exist, return value set to -1, else return value set to the index.
    Operation_Result addOneRecord(messageInfo RecordToStore);
    Operation_Result UpdateOneRecord(messageInfo RecordToUpdate);
    Operation_Result DeleteOneRecord(messageInfo RecordToDelete);
    Operation_Result DeleteRecordsByTelenumber(messageInfo RelatedRecord);

    QString ConstructRecordString(messageInfo RecordToDisplay);
private:
    QString m_DatabaseAlias;
    QString m_TableName;
    messageInfoList m_messageInfoList;
    messageTopInfoList m_messageTopInfoList;
    QSqlDatabase db;
};



#endif // SHORTMESSAGETABLE_H

