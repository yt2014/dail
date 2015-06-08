#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "CContatorsTable.h"

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
    ContactorInfoList m_ContactorInfoList;
    CContactorsTable * m_ContactorTable;
    bool NeedRead_ContactorsInfoAll;
    bool NeedDisplay_ContactorsInfoAll;
};

#endif // MAINWINDOW_H
