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

typedef struct
{
    QString telenum;
    QDateTime startTime;
}CommRecordTopInfo;


typedef QList<CommRecordInfo> CommRecordInfoList;
typedef QList<CommRecordTopInfo> CommRecordTopList;



typedef struct
{
    QString telenum;
    bool existInContactorTable;
}telenumInfo;

Q_DECLARE_METATYPE(telenumInfo)

class CCommRecordTable:CDatabaseTable
{
public:
    CCommRecordTable(QString DatabaseAlias = "Dail",QString TableName = "communicate_record");
    virtual ~CCommRecordTable();
    void setDatabaseAlias(QString DatabaseAlias);
    void setTableName(QString TableName);
    CommRecordInfoList getListAllFromDatabase();
    CommRecordInfoList getListBySql(QString strSql);
    CommRecordTopList getListTop();
    CommRecordTopList getListTopBySql(QString strSql);
    int isUserNameExist(CommRecordInfo RecordToStore);//if not exist, return value set to -1, else return value set to the index.
    bool openDatabase();
    int isTeleNumExistInTopList(QString telenumber,CommRecordTopList topList);//if not exist, return value set to -1, else return value set to the index.
    Operation_Result addOneRecord(CommRecordInfo RecordToStore);
    Operation_Result UpdateOneRecord(CommRecordInfo RecordToUpdate);
    Operation_Result DeleteOneRecord(CommRecordInfo RecordToDelete);
    Operation_Result DeleteRecordsByTelenumber(CommRecordInfo RelatedRecord);

    QString ConstructRecordString(CommRecordInfo RecordToDisplay);
private:
    QString m_DatabaseAlias;
    QString m_TableName;
    CommRecordInfoList m_CommRecordInfoList;
    CommRecordTopList m_CommRecordTopList;
    QSqlDatabase db;
};




#endif // COMMRECORDTABLE_H

