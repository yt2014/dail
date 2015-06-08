#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "CContatorsTable.h"
#include "CommRecordTable.h"
#include <QTreeWidget>


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

private slots:
    void on_tabWidget_currentChanged(int index);
    void ReShowFromTray(QSystemTrayIcon::ActivationReason reason);
    void setWindowMin();
    void closeWindow();

    void on_pBtn_Dailout_clicked();

    void on_pBtn_Contactors_clicked();

    void on_pBtn_Dail_clicked();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * mSystemTrayIcon;    

    /*for contactors table below values*/
    ContactorInfoList m_ContactorInfoList;
    CContactorsTable * m_ContactorTable;
    bool NeedRead_ContactorsInfoAll;
    bool NeedDisplay_ContactorsInfoAll;

    /*for communication record below values*/
    CommRecordInfoList m_CommRecordInfoList;
    CCommRecordTable * m_CCommRecordTable;
    bool NeedRead_CommRecordInfoAll;
    bool NeedDisplay_CommRecordInfoAll;

    /*tree widget value*/
    QTreeWidget * m_CommRecordTree;

};

#endif // MAINWINDOW_H
