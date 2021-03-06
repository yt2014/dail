﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "CContatorsTable.h"
#include "CommRecordTable.h"
#include "ChinesePinyinTable.h"
#include <QTreeWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include "AddContactorThread.h"
#include "DynamicSelectThread.h"
#include "ModemPoolSerialPort.h"
#include "ModemPool.h"
#include "ShortMessageTable.h"

class CDynamicSelectThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual void closeEvent(QCloseEvent *e);
    virtual void changeEvent(QEvent *e);

    void RefreshContent(int index, bool displayAll);//refresh the content of tabwidget.
    void setFreshList(CommRecordTopList commRecordList);
    void setFreshList(ContactorInfoList contactorList);

private slots:
    void on_tabWidget_currentChanged(int index);
    void ReShowFromTray(QSystemTrayIcon::ActivationReason reason);
    void setWindowMin();
    void closeWindow();

    void on_pBtn_Dailout_clicked();

    void on_pBtn_Contactors_clicked();

    void on_pBtn_Dail_clicked();

    void on_pBtnDail_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_pBtnEdit_Add_clicked();

    void on_pBtnCancel_clicked();

    void on_pBtn_EditSave_clicked();
    void updateRecord();

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_pBtn_AddContactor_clicked();

    void on_pBtn_Edit_clicked();

    void on_pBtn_DeleteContactor_clicked();

    void launchShorMessageForm();

    void portsChanged(int index);

    void OpenClosePort();

    void AddNumsProcess();
    void DelNumsProcess();

    void endProcess();
    void startProcess();

    void on_pBtnMessageRecord_clicked();
    void shortMessageTree_ItemClicked(QTreeWidgetItem *item, int column);

    void on_pBtnSendMessage_clicked();

    void teToSendTextChanged();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * mSystemTrayIcon;    

    /*for contactors table below values*/
    ContactorInfoList m_ContactorInfoList;
    CContactorsTable * m_ContactorTable;
    bool NeedRead_ContactorsInfoAll;
    bool NeedDisplay_ContactorsInfoAll;

    /*for communication record below values*/
    CommRecordTopList m_CommRecordTopListAll;
    CCommRecordTable * m_CCommRecordTable;
    bool NeedRead_CommRecordInfoAll;
    bool NeedDisplay_CommRecordInfoAll;
    bool isAddingContactor;
    QTreeWidgetItem * m_treeItemActive;

    /*tree widget value*/
    QTreeWidget * m_CommRecordTree;

    /*chinese pinyin*/
    CChinesePinyinTable * m_CChinesePinyinTable;

    void initCommRecordTab();

    QTimer* m_timer;
//update contactors record of pinyin
    AddContactorThread ThreadAdding;

    //values for refreshing the contactors and communication records when dynamic searching
    CommRecordTopList m_topFreshCommRecordList;
    ContactorInfoList m_ContactorFreshList;
    CDynamicSelectThread * ThreadSearching;

    //for modify contactor info
    ContactorInfo conInfoSelected;

    //modem
    CModemPool * m_Modem;

    QPushButton * pbtn_OpenClose;
    QLabel * label_SIMNumber;

    //adjust the position of widgets(pushbutton and labels)
    void adjustPosition();

    //for short message record table below values
    messageTopInfoList m_messageTopInfoList;
    messageTopInfoList m_messageFreshTopInfoList;
    CShortMessageTable* m_ShortMessageTable;
    bool NeedRead_ShortMessageRecordInfoAll;
    bool NeedDisplay_ShortMessageRecordInfoAll;

    void refreshMessageDisplay(bool displayAll);

    QList<QPushButton *> mainBtnList;

    void setColorForBtns(int indexOfToMark);
    void setEnabledForBtns(bool enabled);
};

#endif // MAINWINDOW_H
