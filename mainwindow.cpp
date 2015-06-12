#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QTextCodec>
#include <QSqlDatabase>
#include <QCloseEvent>
#include <QTimer>
#include <QMutex>

#pragma warning(disable: 4819)

extern QMutex mutex;
extern ContactorInfoList ContactorlistToAdd;

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
    //this->setBaseSize(541,557);
    //this->resize( QSize( 541, 557 ));

    this->setWindowFlags(Qt::FramelessWindowHint);
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
    NeedRead_ContactorsInfoAll = false;
    NeedDisplay_ContactorsInfoAll = true;

    initCommRecordTab();

    isAddingContactor = false;


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
  //  minButton->setStyleSheet("QToolButton{background-color:transparent;}");
     closeButton->setStyleSheet("QToolButton{background-color:transparent;}");

     minButton->setStyleSheet("QToolButton{background-color:transparent;}"
                              "QToolButton:hover{background-color:grey;}");
     closeButton->setStyleSheet("QToolButton{background-color:transparent;}"
                                "QToolButton:hover{background-color:red;}");

     connect(closeButton, SIGNAL(clicked()), this, SLOT(closeWindow()) );
     connect(minButton, SIGNAL(clicked()), this, SLOT(setWindowMin()));
    // connect(maxButton, SIGNAL(clicked()), this, SLOT(winmax()));


     ui->tabWidget->setCurrentIndex(0);
     mSystemTrayIcon = new QSystemTrayIcon(this);
     QIcon icon("tele.ico");

      mSystemTrayIcon->setIcon(icon);

      connect(mSystemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ReShowFromTray(QSystemTrayIcon::ActivationReason)));

      ThreadAdding.start();
}

/*void MainWindow::showMe(){
    show();
    setWindowState(Qt::WindowNoState);
}
*/
void MainWindow::setWindowMin()
{
    ui->label_indications->setText("min button clicked");
    this->showMinimized();
}

void MainWindow::closeWindow()
{
  ui->label_indications->setText("close button clicked");
  this->close();
}

void MainWindow::ReShowFromTray(QSystemTrayIcon::ActivationReason reason)
{
    this->show();
    setWindowState(Qt::WindowNoState);
}


MainWindow::~MainWindow()
{

    delete mSystemTrayIcon;
    delete m_ContactorTable;
    delete m_CCommRecordTable;
    delete m_CChinesePinyinTable;
    ThreadAdding.stop();
    delete ui;

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
          if(NeedRead_ContactorsInfoAll == true)
          {
              m_ContactorInfoList = m_ContactorTable->getListAllFromDatabase();
              NeedRead_ContactorsInfoAll = false;
          }
          if(NeedDisplay_ContactorsInfoAll)
          {
               ui->listWidget->clear();
               int num_ToAdd = m_ContactorInfoList.count();
               QListWidgetItem * itemToAdd;

               for(int i=0;i<num_ToAdd;i++)
               {
                    ContactorInfo oneRecord = m_ContactorInfoList.at(i);
                    QString str_ToAdd = oneRecord.name + " " + oneRecord.telenum;
                    itemToAdd = new QListWidgetItem(str_ToAdd);

                     ui->listWidget->insertItem(0,itemToAdd);
               }
               NeedDisplay_ContactorsInfoAll = false;
          }

    }
        break;
    case 1:
        {
           CommRecordTopList temList;
           CommRecordInfoList tempListOneNum;
           if(NeedRead_CommRecordInfoAll == true)
           {

               temList = m_CCommRecordTable->getListTop();
               NeedRead_CommRecordInfoAll = false;
           }
           if(NeedDisplay_CommRecordInfoAll)
           {
               ui->treeWidget->clear();
               int num_ToAdd = temList.count();

               QString str_sql_begin = "select * from communicate_record where telenumber = \'";
               QString str_sql_end = "\' order by startTime DESC";
               QString str_sql;

               int j=0;
               int numOneTeleNum;
               for(int i=0;i<num_ToAdd;i++)
               {
                    CommRecordTopInfo oneTopRecord = temList.at(i);
                    str_sql = str_sql_begin + oneTopRecord.telenum + str_sql_end;
                    tempListOneNum = m_CCommRecordTable->getListBySql(str_sql);
                    numOneTeleNum = tempListOneNum.count();
                    QTreeWidgetItem * ItemToAdd;
                    CommRecordInfo oneFullRecord;
                    for(j=0;j<numOneTeleNum;j++)
                    {
                        oneFullRecord = tempListOneNum.at(j);

                        if(j==0)
                        {
                            QString str_SelectContactor = "select * from contactors where telenumber = \'" + oneFullRecord.telenum + "\'";

                            ContactorInfoList tempContactor =  m_ContactorTable->getListBySql(str_SelectContactor);

                            QString strName = "";

                            telenumInfo infoToAdd;

                            infoToAdd.telenum = oneFullRecord.telenum;


                            if(tempContactor.count()==0)
                            {
                                strName = "无题名";
                                infoToAdd.existInContactorTable = false;
                            }
                            else
                            {
                                strName = tempContactor.at(0).name;
                                infoToAdd.existInContactorTable = true;
                            }

                            QVariant valueToAdd;
                            valueToAdd.setValue(infoToAdd);

                            QStringList strList = QStringList()<<oneFullRecord.telenum+" "+strName;
                            ItemToAdd = new QTreeWidgetItem(strList);
                            ItemToAdd->setData(0, Qt::UserRole,	valueToAdd);
                            m_CommRecordTree->addTopLevelItem(ItemToAdd);

                            QString str_record = m_CCommRecordTable->ConstructRecordString(oneFullRecord);

                            strList = QStringList()<<str_record;

                            QTreeWidgetItem * childItemToAdd = new QTreeWidgetItem(strList);

                            childItemToAdd->setToolTip(0,str_record);

                            ItemToAdd->addChild(childItemToAdd);
                        }
                        else
                        {
                            QString str_record = m_CCommRecordTable->ConstructRecordString(oneFullRecord);
                            QStringList strList = QStringList()<<str_record;
                            QTreeWidgetItem * childItemToAdd = new QTreeWidgetItem(strList);

                            childItemToAdd->setToolTip(0,str_record);
                            ItemToAdd->addChild(childItemToAdd);
                        }
                    }
               }
              NeedDisplay_CommRecordInfoAll = false;
           }

        }
        break;
    case 2:
        break;
    case 3:
        break;
    default:
        break;


    }
}

void MainWindow::on_pBtn_Dailout_clicked()
{
    ui->label_indications->setText("拨号中。。。");
}

void MainWindow::on_pBtn_Contactors_clicked()
{
    //ui->tabWidget->currentChanged(0);
    ui->tabWidget->setCurrentIndex(0);
}


void MainWindow::on_pBtn_Dail_clicked()
{

    //ui->tabWidget->currentChanged(1);
    ui->tabWidget->setCurrentIndex(1);

}

void MainWindow::on_pBtnDail_clicked()
{
     //m_CChinesePinyinTable->initTable();
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    m_treeItemActive = item;
    QString strDisplay = QString::number(column) + " clicked";

    QTreeWidgetItem *parentItem = item->parent();

    telenumInfo infoFromItem;

    if(parentItem)
    {
        //child
        infoFromItem = parentItem->data(0,Qt::UserRole).value<telenumInfo>();
    }
    else
    {
        //parent
        infoFromItem = item->data(0,Qt::UserRole).value<telenumInfo>();
    }

    strDisplay = infoFromItem.telenum;

    if(infoFromItem.existInContactorTable)
    {
       ui->pBtnEdit_Add->setEnabled(false);
    }
    else
    {
        ui->pBtnEdit_Add->setEnabled(true);
    }

    ui->label_Telenumber->setText(strDisplay);
}

void MainWindow::initCommRecordTab()
{
    ui->labelInputName->hide();
    ui->lineEdit_InputName->hide();
    ui->pBtn_EditSave->hide();
    ui->pBtnCancel->hide();

    ui->label_Telenumber->setText("");

    m_CCommRecordTable = new CCommRecordTable();
    m_CommRecordInfoList = m_CCommRecordTable->getListAllFromDatabase();
    NeedRead_CommRecordInfoAll = true;
    NeedDisplay_CommRecordInfoAll = true;
    m_CommRecordTree = ui->treeWidget;
    ui->treeWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    //ui->treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_CommRecordTree->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QStringList headers;
    headers << QObject::tr("通话记录");
    m_CommRecordTree->setHeaderLabels(headers);

    m_CChinesePinyinTable = new CChinesePinyinTable();
}


void MainWindow::on_pBtnEdit_Add_clicked()
{
    ui->labelInputName->show();
    ui->lineEdit_InputName->show();
    ui->pBtn_EditSave->show();
    ui->pBtnCancel->show();

    ui->pBtn_EditSave->setText("添加");

    ui->pBtnEdit_Add->hide();
    ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);
    isAddingContactor = true;
}

void MainWindow::on_pBtnCancel_clicked()
{
    ui->labelInputName->hide();
    ui->lineEdit_InputName->hide();
    ui->pBtn_EditSave->hide();
    ui->pBtnCancel->hide();

    ui->pBtnEdit_Add->show();
    ui->pBtnDail->show();

    ui->treeWidget->setEnabled(true);

    isAddingContactor = false;
}

void MainWindow::on_pBtn_EditSave_clicked()
{

    if(isAddingContactor)
    {

       /*save data to database here*/
       ContactorInfo infoToAdd;
       infoToAdd.name = ui->lineEdit_InputName->text().trimmed();
       infoToAdd.telenum = ui->label_Telenumber->text().trimmed();

       if(infoToAdd.name.isEmpty()||(infoToAdd.name==tr("请在此输入名字")))
       {
           ui->lineEdit_InputName->setText("请在此输入名字");
       }
       else
       {
          ui->pBtn_EditSave->setText("确定");

          QRect positionBtn = ui->pBtn_EditSave->geometry();

          // positionBtn.moveRight(2);
           //positionBtn.width();

          positionBtn.adjust(20,0,20,0);
          ui->pBtn_EditSave->setGeometry(positionBtn);
          ui->pBtnCancel->hide();
          isAddingContactor = false;
          Operation_Result AddResult = m_ContactorTable->addOneRecord(infoToAdd);

           if( AddResult == AddSuccess)
           {
               QString strResult = "添加成功";
               mutex.lock();
               ContactorlistToAdd.append(infoToAdd);
               mutex.unlock();
               ui->label_Telenumber->setText(strResult);

           }
           else
           {
               QString strResult = "add result is " + QString::number(AddResult);
               ui->label_Telenumber->setText(strResult);
           }
       }
       /*then display the adding result*/
    }
    else
    {

       ui->pBtn_EditSave->setText("添加");
       ui->labelInputName->hide();
       ui->lineEdit_InputName->hide();
       ui->pBtn_EditSave->hide();
       ui->pBtnCancel->hide();

       ui->pBtnEdit_Add->show();
       ui->pBtnDail->show();
       ui->treeWidget->setEnabled(true);

       ui->label_Telenumber->setText("");

       NeedDisplay_CommRecordInfoAll = true;
       NeedRead_CommRecordInfoAll = true;
       NeedDisplay_ContactorsInfoAll = true;
       NeedRead_ContactorsInfoAll = true;
       ui->tabWidget->setCurrentIndex(0);

     /*  m_timer = new QTimer(this);
       connect(m_timer, SIGNAL(timeout()), this, SLOT(updateRecord()));
       m_timer->start(500);
       */
       QTimer::singleShot(100, this, SLOT(updateRecord()));

       }
}

void MainWindow::updateRecord()
{
    ui->tabWidget->setCurrentIndex(1);
}
