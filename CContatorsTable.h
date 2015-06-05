#ifndef CCONTATORSTABLE_H
#define CCONTATORSTABLE_H

#include "DatabaseTable.h"
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

typedef QList<ContactorInfo> ContactorInfoList;


class CContactorsTable:CDatabaseTable
{
public:
    CContactorsTable(QString DatabaseAlias = "Dail",QString TableName = "contractors");
    virtual ~CContactorsTable();
    void setDatabaseAlias(QString DatabaseAlias);
    void setTableName(QString TableName);
    ContactorInfoList getListAllFromDatabase();
    int isUserNameExist(ContactorInfo RecordToStore);//if not exist, return value set to -1, else return value set to the index.
    bool openDatabase();
    Operation_Result addOneRecord(ContactorInfo RecordToStore);
    Operation_Result UpdateOneRecord(ContactorInfo RecordToUpdate);
    Operation_Result DeleteOneRecord(ContactorInfo RecordToDelete);
private:
    QString m_DatabaseAlias;
    QString m_TableName;
    ContactorInfoList m_ContactorInfoList;
    QSqlDatabase db;
};



#endif // CCONTATORSTABLE_H
