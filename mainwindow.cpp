#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QTextCodec>
#include <QSqlDatabase>
#include <QCloseEvent>
#include <QTimer>

#pragma warning(disable: 4819)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPalette palette;
   // palette.setColor(QPalette::Background, QColor(192,253,123));
    palette.setBrush(QPalette::Background, QBrush(QPixmap("background.jpg")));
    this->setPalette(palette);

    QFont font;
    font.setBold(true);
    font.setFamily("Courier");
    font.setPointSize(14);

    this->setFont(font);
    this->setPalette(palette);
    this->setAutoFillBackground(true);
    this->setFixedSize(541,557);

   // this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowOpacity(1);
 //   this->setWindowFlags(Qt::WindowMinimizeButtonHint);

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);
    //QString  textSet = QString::fromUtf8("联系人");

  //  QString Str_Set = codec->convertToUnicode(textSet);

    QToolButton *tBtn_Contractors = new QToolButton;
    tBtn_Contractors->setText(QStringLiteral("联系人"));
    QToolButton *tBtn_Dail = new QToolButton;
    tBtn_Dail->setText(QStringLiteral("通话记录"));

    ui->mainToolBar->addWidget(tBtn_Contractors);
    ui->mainToolBar->addWidget(tBtn_Dail);

    ui->mainToolBar->hide();
    //this->ui->pBtn_Contactors->setFlat(1);
    //this->ui->pBtn_Dail->setFlat(1);
    //this->ui->pBtn_Dail->setStyleSheet("background-color:transparent;");
    font.setPointSize(10);

    ui->tabWidget->setFont(font);

    QSqlDatabase db = QSqlDatabase::database("Dail",true);

    if(db.isValid()&&db.isOpen())
    {
        ui->label_indications->setText("database opened successfully!");
     //   ui->label_indications->setText("");
    }
    else
    {
        ui->label_indications->setText("can't open database!");
    }

    m_ContactorTable = new CContactorsTable();

    m_ContactorInfoList = m_ContactorTable->getListAllFromDatabase();

    int width = this->width();//获取界面的宽度

     //构建最小化、最大化、关闭按钮
     QToolButton *minButton = new QToolButton(this);
     QToolButton *closeButton= new QToolButton(this);

     //获取最小化、关闭按钮图标
     QPixmap minPix  = style()->standardPixmap(QStyle::SP_TitleBarMinButton);
     QPixmap closePix = style()->standardPixmap(QStyle::SP_TitleBarCloseButton);

     //设置最小化、关闭按钮图标
     minButton->setIcon(minPix);
     closeButton->setIcon(closePix);
     //设置最小化、关闭按钮在界面的位置
     minButton->setGeometry(width-46,5,20,20);
     closeButton->setGeometry(width-25,5,20,20);

     //设置鼠标移至按钮上的提示信息
     minButton->setToolTip(tr("最小化"));
     closeButton->setToolTip(tr("关闭"));
     //设置最小化、关闭按钮的样式
     minButton->setStyleSheet("background-color:transparent;");
     closeButton->setStyleSheet("background-color:transparent;");

     ui->tabWidget->currentChanged(0);
     mSystemTrayIcon = new QSystemTrayIcon(this);
     QIcon icon("tele.ico");

      mSystemTrayIcon->setIcon(icon);

      connect(mSystemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ReShowFromTray(QSystemTrayIcon::ActivationReason)));
}

/*void MainWindow::showMe(){
    show();
    setWindowState(Qt::WindowNoState);
}
*/
void MainWindow::ReShowFromTray(QSystemTrayIcon::ActivationReason reason)
{
    this->show();
    setWindowState(Qt::WindowNoState);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete mSystemTrayIcon;
    delete m_ContactorTable;
}

//关闭到托盘---------
void MainWindow::closeEvent(QCloseEvent *e)
{
//    e->ignore();
//    this->hide();
}


//最小化到托盘----
void MainWindow::changeEvent(QEvent *e)
{
    if(e->type()==QEvent::WindowStateChange){
            if(windowState() & Qt::WindowMinimized){
               QTimer::singleShot(0, this, SLOT(hide()));

               mSystemTrayIcon->show();
            }
        QMainWindow::changeEvent(e);
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    QString strDisplay = QString::number(index) + "is selected";

    ui->label_indications->setText(strDisplay);

    switch(index)
    {
    case 0:
    {
          int num_ToAdd = m_ContactorInfoList.count();
          QListWidgetItem * itemToAdd;

          for(int i=0;i<num_ToAdd;i++)
          {
              ContactorInfo oneRecord = m_ContactorInfoList.at(i);
              QString str_ToAdd = oneRecord.name +  oneRecord.telenum;
              itemToAdd = new QListWidgetItem(str_ToAdd);

              ui->listWidget->insertItem(0,itemToAdd);
          }


    }
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    default:
        break;


    }
}
