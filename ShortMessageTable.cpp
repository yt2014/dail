#include "ShortMessageTable.h"

CShortMessageTable::CShortMessageTable(QString DatabaseAlias = "Dail",QString TableName = "shortMessage")
{
    m_DatabaseAlias = DatabaseAlias;
    m_TableName = TableName;
}


CShortMessageTable::~CShortMessageTable()
{
    if(m_messageInfoList.count()!=0)
    {
       m_messageInfoList.clear();
    }

    if(m_messageTopInfoList.count()!=0)
    {
        m_messageTopInfoList.clear();
    }

    if(db.isOpen())
    {
        db.close();
    }
}

void CShortMessageTable::setDatabaseAlias(QString DatabaseAlias)
{
     m_DatabaseAlias = DatabaseAlias;
}

void CShortMessageTable::setTableName(QString TableName)
{
    m_TableName = TableName;
}

messageInfoList CShortMessageTable::getListAllFromDatabase()
{
    if(m_messageInfoList.count()!=0)
    {
         m_messageInfoList.clear();
    }
    if(openDatabase())
    {
        QSqlQuery query(db);

        QString strSelect = "select * from " + m_TableName;
        if(query.exec(strSelect))
        {
               QSqlRecord columns = query.record();

               int index_numRemote = columns.indexOf("NumberRemote");
               int index_numLocal =  columns.indexOf("NumberLocal");
               int index_send_recvTime = columns.indexOf("send_recvTime");
               int index_Received = columns.indexOf("Receive_Send");
               int index_context  = columns.indexOf("context");
               int index_subID = columns.indexOf("subID");

               while(query.next())
               {
                   messageInfo oneinfo;
                   oneinfo.isReceived = query.value(index_Received).toBool();
                   oneinfo.send_recvTime = query.value(index_send_recvTime).toDateTime();
                   oneinfo.NumberLocal = query.value(index_numLocal).toString();
                   oneinfo.NumberRemote = query.value(index_numRemote).toString();
                   oneinfo.messageContext = query.value(index_context).toString();
                   oneinfo.subID = query.value(index_subID).toInt();

                   m_messageInfoList.append(oneinfo);

               }
        }
        return m_messageInfoList;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return m_messageInfoList;
    }
}

messageInfoList CShortMessageTable::getListBySql(QString strSql)
{
    messageInfoList tempList;

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

            int index_numRemote = columns.indexOf("NumberRemote");
            int index_numLocal =  columns.indexOf("NumberLocal");
            int index_send_recvTime = columns.indexOf("send_recvTime");
            int index_Received = columns.indexOf("Receive_Send");
            int index_context  = columns.indexOf("context");
            int index_subID = columns.indexOf("subID");

            while(query.next())
            {
                messageInfo oneinfo;
                oneinfo.isReceived = query.value(index_Received).toBool();
                oneinfo.send_recvTime = query.value(index_send_recvTime).toDateTime();
                oneinfo.NumberLocal = query.value(index_numLocal).toString();
                oneinfo.NumberRemote = query.value(index_numRemote).toString();
                oneinfo.messageContext = query.value(index_context).toString();
                oneinfo.subID = query.value(index_subID).toInt();

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

messageTopInfoList CShortMessageTable::getListTop()
{
    messageTopInfoList tempList;

    if(tempList.count()!=0)
    {
         tempList.clear();
         m_messageTopInfoList.clear();
    }
    if(openDatabase())
    {
        QSqlQuery query(db);

        QString strSQL = "select max(send_recvTime) as m_send_receiveTime,NumberRemote from shortMessage ";
                strSQL += "group by NumberRemote ";
                strSQL += "order by max(send_recvTime) DESC";

        if(query.exec(strSQL))
        {
               QSqlRecord columns = query.record();

               int index_telenum = columns.indexOf("NumberRemote");
               int index_send_recvTime = columns.indexOf("m_send_receiveTime");


               while(query.next())
               {
                   messageTopInfo oneinfo;
                   oneinfo.telenum = query.value(index_telenum).toString();
                   oneinfo.telenum = query.value(index_send_recvTime).toDateTime();

                   tempList.append(oneinfo);

               }
        }
        m_messageTopInfoList = tempList;
        return tempList;
    }
    else
    {
      //  QMessageBox::warning(this,QObject::tr("warning"),QObject::tr("can't open database!"),QMessageBox::Ok);
        qDebug()<<"can't open database!";
        return tempList;
    }
}

messageTopInfoList CShortMessageTable::getListTopBySql(QString strSql)
{
    messageTopInfoList tempList;

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

               int index_telenum = columns.indexOf("NumberRemote");
               int index_send_recvTime = columns.indexOf("m_send_receiveTime");


               while(query.next())
               {
                   messageTopInfo oneinfo;
                   oneinfo.telenum = query.value(index_telenum).toString();
                   oneinfo.send_recvTime = query.value(index_send_recvTime).toDateTime();

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

bool CShortMessageTable::openDatabase()
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

int CShortMessageTable::isRecordExist(messageInfo RecordToSearch)
{
    QString numToFind =  RecordToSearch.NumberRemote;
    QDateTime timeToFind = RecordToSearch.send_recvTime;

    int num_records = m_messageInfoList.count();
    int i=-1;
    for(i=0;i<num_records;i++)
    {
        if((m_messageInfoList.at(i).NumberRemote == numToFind)&&
           (timeToFind==m_messageInfoList.at(i).send_recvTime)
           )
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

int CShortMessageTable::isTeleNumExistInTopList(QString telenumber,messageTopInfoList topList)
{
    int num_records = topList.count();
    int i=-1;
    for(i=0;i<num_records;i++)
    {
        if(topList.at(i).telenum == telenumber)
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

Operation_Result CShortMessageTable::addOneRecord(messageInfo RecordToStore)
{
    Operation_Result value_ret = AddFailed;

     if(!db.isOpen())
     {
        value_ret =  DataBaseNotOpen;
     }
     else
     {

             QSqlQuery query(db);

             QString strSQL = "insert into " + m_TableName + " (subID,send_recvTime,NumberRemote,NumberLocal,Receive_Send,context) values ("
                                                           + QString::number(RecordToStore.subID) + ",\'"
                                                           + RecordToStore.send_recvTime.toString("yyyy-MM-dd hh:mm:ss") + "\',\'"
                                                           + RecordToStore.NumberRemote + "\',\'"
                                                           + RecordToStore.NumberLocal + "\',"
                                                           + QString::number(RecordToStore.isReceived) + ",\'"
                                                           + RecordToStore.messageContext + "\')";
            // qDebug()<<strSQL;
            if(query.exec(strSQL))
            {
                value_ret = AddSuccess;
            }
            else
            {
                value_ret = AddFailed;
            }

     }

return value_ret;

}

Operation_Result CShortMessageTable::UpdateOneRecord(messageInfo RecordToUpdate)
{
    //make no sense to use this function.
    Operation_Result value_ret = UpdateFailed;

    int index_to_update;

    if(!db.isOpen())
    {
       value_ret =  DataBaseNotOpen;
    }
    else
    {
        index_to_update = isRecordExist(RecordToUpdate);
        if(index_to_update == -1)
        {
            value_ret = UpdateFailed;
        }
        else
        {
            QSqlQuery query(db);

            QString strSQL = "update " + m_TableName + " set telenumber = '" + RecordToUpdate.telenum
                                                     +" where UserName = '" + RecordToUpdate.startTime.toString()
                                                     +"'";
           if(query.exec(strSQL))
           {
               value_ret = UpdateSuccess;
               m_messageInfoList.removeAt(index_to_update);
               m_messageInfoList.insert(index_to_update,RecordToUpdate);
           }
           else
           {
               value_ret = UpdateFailed;
           }

        }

    }

    return value_ret;
}

Operation_Result CShortMessageTable::DeleteOneRecord(messageInfo RecordToDelete)
{
    Operation_Result value_ret = DeleteFailed;

    int index_to_delete;

    if(!db.isOpen())
    {
       value_ret =  DataBaseNotOpen;
    }
    else
    {
        index_to_delete = isRecordExist(RecordToDelete);
        if(index_to_delete == -1)
        {
            value_ret = DeleteNotExistRecord;
        }
        else
        {
            QSqlQuery query(db);

            QString strSQL = "delete from " + m_TableName + " where NumberRemote = \'" + RecordToDelete.NumberRemote
                                                          + "\' and send_recvTime = \'"
                                                          + RecordToDelete.send_recvTime.toString("yyyy-MM-dd hh:mm:ss")
                                                          +"\'";
           if(query.exec(strSQL))
           {
               value_ret = DeleteSuccess;
               m_messageInfoList.removeAt(index_to_delete);
           }
           else
           {
               value_ret = DeleteFailed;
           }

        }

    }

    return value_ret;
}

Operation_Result CShortMessageTable::DeleteRecordsByTelenumber(messageInfo RecordToDelete)
{
    Operation_Result value_ret = DeleteFailed;

    int index_to_delete;

    if(!db.isOpen())
    {
       value_ret =  DataBaseNotOpen;
    }
    else
    {
        index_to_delete = isTeleNumExistInTopList(RecordToDelete.NumberRemote,m_messageTopInfoList);
        if(index_to_delete == -1)
        {
            value_ret = DeleteNotExistRecord;
        }
        else
        {
            QSqlQuery query(db);

            QString strSQL = "delete from " + m_TableName + " where NumberRemote = \'" + RecordToDelete.NumberRemote
                                                          + "\'";
           if(query.exec(strSQL))
           {
               value_ret = DeleteSuccess;
               m_messageTopInfoList.removeAt(index_to_delete);
           }
           else
           {
               value_ret = DeleteFailed;
           }

        }

    }

    return value_ret;
}

QString CShortMessageTable::ConstructRecordString(messageInfo RecordToDisplay)
{
    QString str_record = RecordToDisplay.send_recvTime.toString("yyyy-MM-dd hh:mm:ss");

    if(RecordToDisplay.isReceived)
    {
        str_record += " 来自";

    }
    else
    {
        str_record += " 发到";
    }

    return str_record;
}



