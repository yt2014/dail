#include "CContatorsTable.h"


CContactorsTable::CContactorsTable(QString DatabaseAlias,QString TableName)
{
    m_DatabaseAlias = DatabaseAlias;
    m_TableName = TableName;
    m_ContactorInfoList = ContactorInfoList();
}

CContactorsTable::~CContactorsTable()
{
    if(m_ContactorInfoList.count()!=0)
    {
       m_ContactorInfoList.clear();
    }

    if(db.isOpen())
    {
        db.close();
    }
}

void CContactorsTable::setDatabaseAlias(QString DatabaseAlias)
{
     m_DatabaseAlias = DatabaseAlias;
}

void CContactorsTable::setTableName(QString TableName)
{
    m_TableName = TableName;
}

ContactorInfoList CContactorsTable::getListAllFromDatabase()
{
    if(m_ContactorInfoList.count()!=0)
    {
         m_ContactorInfoList.clear();
    }
    if(openDatabase())
    {
        QSqlQuery query(db);

        QString strSelect = "select * from " + m_TableName;
        if(query.exec(strSelect))
        {
               QSqlRecord columns = query.record();

               int index_Name = columns.indexOf("name");
               int index_telenum = columns.indexOf("telenumber");

               while(query.next())
               {
                   ContactorInfo oneinfo;
                   oneinfo.name = query.value(index_Name).toString();
                   oneinfo.telenum = query.value(index_telenum).toString();

                   m_ContactorInfoList.append(oneinfo);

               }
        }
        return m_ContactorInfoList;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return m_ContactorInfoList;
    }
}

ContactorInfoList getListBySql(QString strSql)
{
    ContactorInfoList listReturn = ContactorInfoList();
    if(openDatabase())
    {
        QSqlQuery query(db);

        if(query.exec(strSql))
        {
               QSqlRecord columns = query.record();

               int index_Name = columns.indexOf("name");
               int index_telenum = columns.indexOf("telenumber");

               while(query.next())
               {
                   ContactorInfo oneinfo;
                   oneinfo.name = query.value(index_Name).toString();
                   oneinfo.telenum = query.value(index_telenum).toString();

                   listReturn.append(oneinfo);

               }
        }
        return listReturn;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return listReturn;
    }
}

bool CContactorsTable::openDatabase()
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

int CContactorsTable::isUserNameExist(ContactorInfo RecordToStore)
{
    QString strToFind =  RecordToStore.name;

    int num_records = m_ContactorInfoList.count();
    int i=0;
    for(i=0;i<num_records;i++)
    {
        if(m_ContactorInfoList.at(i).name == strToFind)
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

int CContactorsTable::isTeleNumExit(QString strToFind)
{
    int retVal = -1;
    //QString strToFind =  oneRecord.telenum;

    int num_records = m_ContactorInfoList.count();
    int i=0;
    for(i=0;i<num_records;i++)
    {
        if(m_ContactorInfoList.at(i).telenum == strToFind)
            break;
    }
    if(i<num_records)
    {
        retVal = i;
    }
    else
    {
        retVal = -1;
    }
    return retVal;

}



Operation_Result CContactorsTable::addOneRecord(ContactorInfo RecordToStore)
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

             QString strSQL = "insert into " + m_TableName + " (UserName,password,permission) values (\'"
                                                              + RecordToStore.name + "\',\'"
                                                              + RecordToStore.telenum + "\')";
            if(query.exec(strSQL))
            {
                value_ret = AddSuccess;
                m_ContactorInfoList.append(RecordToStore);
            }
            else
            {
                value_ret = AddFailed;
            }

         }

     }

return value_ret;

}
Operation_Result CContactorsTable::UpdateOneRecord(ContactorInfo RecordToUpdate)
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
               m_ContactorInfoList.removeAt(index_to_update);
               m_ContactorInfoList.insert(index_to_update,RecordToUpdate);
           }
           else
           {
               value_ret = UpdateFailed;
           }

        }

    }

    return value_ret;
}
Operation_Result CContactorsTable::DeleteOneRecord(ContactorInfo RecordToDelete)
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
               m_ContactorInfoList.removeAt(index_to_delete);
           }
           else
           {
               value_ret = DeleteFailed;
           }

        }

    }

    return value_ret;
}




