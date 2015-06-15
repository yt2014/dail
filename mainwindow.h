#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "CContatorsTable.h"
#include "CommRecordTable.h"
#include "ChinesePinyinTable.h"
#include <QTreeWidget>
#include "AddContactorThread.h"
#include "DynamicSelectThread.h"

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
};

#endif // MAINWINDOW_H
