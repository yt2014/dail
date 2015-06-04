#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual void MainWindow::closeEvent(QCloseEvent *e);
    virtual void MainWindow::changeEvent(QEvent *e);

private slots:
    void on_tabWidget_currentChanged(int index);
    void ReShowFromTray(QSystemTrayIcon::ActivationReason reason);

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon * mSystemTrayIcon;
};

#endif // MAINWINDOW_H
