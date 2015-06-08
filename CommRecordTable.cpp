#include "CommRecordTable.h"


CCommRecordTable::CCommRecordTable(QString DatabaseAlias,QString TableName)
{
    m_DatabaseAlias = DatabaseAlias;
    m_TableName = TableName;
    m_CommRecordInfoList = CommRecordInfoList();
}

CCommRecordTable::~CCommRecordTable()
{
    if(m_CommRecordInfoList.count()!=0)
    {
       m_CommRecordInfoList.clear();
    }

    if(db.isOpen())
    {
        db.close();
    }
}

void CCommRecordTable::setDatabaseAlias(QString DatabaseAlias)
{
     m_DatabaseAlias = DatabaseAlias;
}

void CCommRecordTable::setTableName(QString TableName)
{
    m_TableName = TableName;
}

CommRecordInfoList CCommRecordTable::getListAllFromDatabase()
{
    if(m_CommRecordInfoList.count()!=0)
    {
         m_CommRecordInfoList.clear();
    }
    if(openDatabase())
    {
        QSqlQuery query(db);

        QString strSelect = "select * from " + m_TableName;
        if(query.exec(strSelect))
        {
               QSqlRecord columns = query.record();

               int index_telenum = columns.indexOf("telenumber");
               int index_startTime = columns.indexOf("startTime");
               int index_duration = columns.indexOf("duration");
               int index_CallIn   = columns.indexOf("come_go");
               int index_CallConnected = columns.indexOf("dail_on");
               int index_RingTimes = columns.indexOf("ring_times");

               while(query.next())
               {
                   CommRecordInfo oneinfo;
                   oneinfo.telenum = query.value(index_telenum).toString();
                   oneinfo.startTime = query.value(index_startTime).toDateTime();
                   oneinfo.callDuration = query.value(index_duration).toInt();
                   oneinfo.isCallIn = query.value(index_CallIn).toBool();
                   oneinfo.isCallConnected = query.value(index_CallConnected).toBool();
                   oneinfo.ringTimes = query.value(index_RingTimes).toInt();

                   m_CommRecordInfoList.append(oneinfo);

               }
        }
        return m_CommRecordInfoList;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return m_CommRecordInfoList;
    }
}

CommRecordInfoList CCommRecordTable::getListBySql(QString strSql)
{
    CommRecordInfoList tempList = new CommRecordInfoList();

    if(tempList.count()!=0)
    {
         tempList.clear();
    }
    if(openDatabase())
    {
        QSqlQuery query(db);

        if(query.exec(strSql))
        {
               QSqlRecord columns = query.record();

               int index_telenum = columns.indexOf("telenumber");
               int index_startTime = columns.indexOf("startTime");
               int index_duration = columns.indexOf("duration");
               int index_CallIn   = columns.indexOf("come_go");
               int index_CallConnected = columns.indexOf("dail_on");
               int index_RingTimes = columns.indexOf("ring_times");

               while(query.next())
               {
                   CommRecordInfo oneinfo;
                   oneinfo.telenum = query.value(index_telenum).toString();
                   oneinfo.startTime = query.value(index_startTime).toDateTime();
                   oneinfo.callDuration = query.value(index_duration).toInt();
                   oneinfo.isCallIn = query.value(index_CallIn).toBool();
                   oneinfo.isCallConnected = query.value(index_CallConnected).toBool();
                   oneinfo.ringTimes = query.value(index_RingTimes).toInt();

                   tempList.append(oneinfo);

               }
        }
        return tempList;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return tempList;
    }
}

CommRecordTopList CCommRecordTable::getListTop()
{
    CommRecordTopList tempList = new CommRecordTopList();

    if(tempList.count()!=0)
    {
         tempList.clear();
    }
    if(openDatabase())
    {
        QSqlQuery query(db);

        QString strSQL = "select max(startTime) telenum from communicate_record " +
                         "group by telenum "+
                         "order by startTime DESC";

        if(query.exec(strSQL))
        {
               QSqlRecord columns = query.record();

               int index_telenum = columns.indexOf("telenumber");
               int index_startTime = columns.indexOf("startTime");


               while(query.next())
               {
                   CommRecordTopInfo oneinfo;
                   oneinfo.telenum = query.value(index_telenum).toString();
                   oneinfo.startTime = query.value(index_startTime).toDateTime();

                   tempList.append(oneinfo);

               }
        }
        return tempList;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return tempList;
    }
}

bool CCommRecordTable::openDatabase()
{
    db = QSqlDatabase::database(m_DatabaseAlias,true);

    if(db.isValid()&&db.isOpen())
    {
        return true;
    }
    else
    {
        return false;
    }
}

int CCommRecordTable::isUserNameExist(CommRecordInfo RecordToStore)
{
    QString strToFind =  RecordToStore.name;

    int num_records = m_CommRecordInfoList.count();
    int i=0;
    for(i=0;i<num_records;i++)
    {
        if(m_CommRecordInfoList.at(i).name == strToFind)
            break;
    }
    if(i<num_records)
    {
        return i;
    }
    else
    {
        return -1;
    }

}

Operation_Result CCommRecordTable::addOneRecord(CommRecordInfo RecordToStore)
{
    Operation_Result value_ret = AddFailed;



     if(!db.isOpen())
     {
        value_ret =  DataBaseNotOpen;
     }
     else
     {
         if(isUserNameExist(RecordToStore) != -1)
         {
             value_ret = AddExistRecord;
         }
         else
         {
             QSqlQuery query(db);

             QString strSQL = "insert into " + m_TableName + " (,password,permission) values (\'"
                                                              + RecordToStore.name + "\',\'"
                                                              + RecordToStore.telenum + "\')";
            if(query.exec(strSQL))
            {
                value_ret = AddSuccess;
                m_CommRecordInfoList.append(RecordToStore);
            }
            else
            {
                value_ret = AddFailed;
            }

         }

     }

return value_ret;

}
Operation_Result CCommRecordTable::UpdateOneRecord(CommRecordInfo RecordToUpdate)
{
    Operation_Result value_ret = UpdateFailed;

    int index_to_update;

    if(!db.isOpen())
    {
       value_ret =  DataBaseNotOpen;
    }
    else
    {
        index_to_update = isUserNameExist(RecordToUpdate);
        if(index_to_update == -1)
        {
            value_ret = UpdateFailed;
        }
        else
        {
            QSqlQuery query(db);

            QString strSQL = "update " + m_TableName + " set telenumber = '" + RecordToUpdate.telenum
                                                     +" where UserName = '" + RecordToUpdate.name
                                                     +"'";
           if(query.exec(strSQL))
           {
               value_ret = UpdateSuccess;
               m_CommRecordInfoList.removeAt(index_to_update);
               m_CommRecordInfoList.insert(index_to_update,RecordToUpdate);
           }
           else
           {
               value_ret = UpdateFailed;
           }

        }

    }

    return value_ret;
}
Operation_Result CCommRecordTable::DeleteOneRecord(CommRecordInfo RecordToDelete)
{
    Operation_Result value_ret = DeleteFailed;

    int index_to_delete;

    if(!db.isOpen())
    {
       value_ret =  DataBaseNotOpen;
    }
    else
    {
        index_to_delete = isUserNameExist(RecordToDelete);
        if(index_to_delete == -1)
        {
            value_ret = DeleteNotExistRecord;
        }
        else
        {
            QSqlQuery query(db);

            QString strSQL = "delete from " + m_TableName + " where UserName = '" + RecordToDelete.name
                                                        +"'";
           if(query.exec(strSQL))
           {
               value_ret = DeleteSuccess;
               m_CommRecordInfoList.removeAt(index_to_delete);
           }
           else
           {
               value_ret = DeleteFailed;
           }

        }

    }

    return value_ret;
}
