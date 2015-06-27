#ifndef CCONTATORSTABLE_H
#define CCONTATORSTABLE_H

#include "DatabaseTable.h"
#include "ChinesePinyinTable.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDebug>


typedef struct
{
    QString name;
    QString telenum;
}ContactorInfo;

Q_DECLARE_METATYPE(ContactorInfo)


typedef enum
{
    AddContactor,
    DeleteContacor,
    ModifyContactor,
    OperationFinished
}ContactorTableOperation;

typedef QList<ContactorInfo> ContactorInfoList;

extern ContactorInfoList ContactorlistToAdd;


class CContactorsTable:CDatabaseTable
{
public:
    CContactorsTable(QString DatabaseAlias = "Dail",QString TableName = "contactors");
    virtual ~CContactorsTable();
    void setDatabaseAlias(QString DatabaseAlias);
    void setTableName(QString TableName);
    ContactorInfoList getListAllFromDatabase();
    ContactorInfoList getListBySql(QString strSql);
    int isUserNameExist(ContactorInfo RecordToStore);//if not exist, return value set to -1, else return value set to the index.
    int isTeleNumExit(QString strNumber);
    bool openDatabase();
    Operation_Result addOneRecord(ContactorInfo RecordToStore);
    Operation_Result UpdateOneRecord(ContactorInfo RecordToUpdate);
    Operation_Result UpdateOneRecord(ContactorInfo newRecord,ContactorInfo oldRecord);
    Operation_Result DeleteOneRecord(ContactorInfo RecordToDelete);
    Operation_Result InsertPinyinForRecord(ContactorInfo RecordToStore);

    void setOperation(ContactorTableOperation OperationFlag);

    ContactorTableOperation getOperation();

    static int isTeleNumExitInList(QString strNumber,QStringList numList);
private:
    QString m_DatabaseAlias;
    QString m_TableName;
    ContactorInfoList m_ContactorInfoList;
    QSqlDatabase db;
    CChinesePinyinTable* m_CChinesePinyinTable;

    ContactorTableOperation Operation;
};



#endif // CCONTATORSTABLE_H
