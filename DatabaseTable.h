#ifndef DATABASETABLE_H
#define DATABASETABLE_H


typedef enum
{
    Data_ADD,
    Data_UPDATE,
    Data_REDUCE
}Change_Operation;

typedef enum
{
    Operation_Success,
    DataBaseNotOpen,
    AddExistRecord,
    AddFailed,
    AddSuccess,
    UpdateFailed,
    UpdateSuccess,
    DeleteNotExistRecord,
    DeleteFailed,
    DeleteSuccess

}Operation_Result;

class CDatabaseTable
{
    Operation_Result addOneRecord(void);
    Operation_Result UpdateOneRecord(void);
    Operation_Result DeleteOneRecord(void);
};

#endif // DATABASETABLE_H
