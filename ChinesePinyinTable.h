#ifndef CHINESEPINYIN_H
#define CHINESEPINYIN_H

#include "DatabaseTable.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDebug>


typedef struct h2p {
  QString py;
  QString py_shengdiao;
  int shengdiao;
  QString hz;
} pyhz_tab;

typedef QList<pyhz_tab> pyhz_tabList;


class CChinesePinyinTable:CDatabaseTable
{
public:
    CChinesePinyinTable(QString DatabaseAlias = "Dail",QString TableName = "pinyin_ChineseCharactor");
    virtual ~CChinesePinyinTable();
    void setDatabaseAlias(QString DatabaseAlias);
    void setTableName(QString TableName);
  //  CommRecordInfoList getListAllFromDatabase();
    bool initTable();
    pyhz_tabList getListBySql(QString strSql);
    bool openDatabase();
    Operation_Result addOneRecord(pyhz_tab RecordToStore);
private:
    QString m_DatabaseAlias;
    QString m_TableName;
    QSqlDatabase db;
};


#endif
