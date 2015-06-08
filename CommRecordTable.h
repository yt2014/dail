#ifndef COMMRECORDTABLE_H
#define COMMRECORDTABLE_H

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
    QString telenum;
    QDateTime startTime;
    int callDuration;
    bool isCallIn;
    bool isCallConnected;
    int ringTimes;
}CommRecordInfo;

typedef QList<CommRecordInfo> CommRecordInfoList;


class CCommRecordTable:CDatabaseTable
{
public:
    CCommRecordTable(QString DatabaseAlias = "Dail",QString TableName = "communicate_record");
    virtual ~CCommRecordTable();
    void setDatabaseAlias(QString DatabaseAlias);
    void setTableName(QString TableName);
    CommRecordInfoList getListAllFromDatabase();
    int isUserNameExist(ContactorInfo RecordToStore);//if not exist, return value set to -1, else return value set to the index.
    bool openDatabase();
    Operation_Result addOneRecord(CommRecordInfo RecordToStore);
    Operation_Result UpdateOneRecord(CommRecordInfo RecordToUpdate);
    Operation_Result DeleteOneRecord(CommRecordInfo RecordToDelete);
private:
    QString m_DatabaseAlias;
    QString m_TableName;
    CommRecordInfoList m_CommRecordInfoList;
    QSqlDatabase db;
};




#endif // COMMRECORDTABLE_H

