#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QTextCodec>
#include <QSqlDatabase>
#include <QCloseEvent>
#include <QTimer>
#include <QMutex>
#include <QComboBox>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QDialog>

#pragma warning(disable: 4819)

extern QMutex mutex;
extern ContactorInfoList ContactorlistToAdd;

//positions in comm record tab
QRect positionEditInputName;// = ui->pBtn_EditSave->geometry();
QRect positionLabelInputName;
QRect positionBtnCancel;
QRect positionBtnEdit;


//positions in contactors tab
QRect positionEditInputName_t;// = ui->pBtn_EditSave->geometry();
QRect positionLabelInputName_t;
QRect positionBtnCancel_t;
QRect positionBtnEdit_t;

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
    ui->label_InputTeleNumber->hide();
    ui->lineEdit_InputTeleNumber->hide();

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

     positionEditInputName = ui->lineEdit_InputName->geometry();
     positionLabelInputName = ui->labelInputName->geometry();
     positionBtnCancel = ui->pBtnCancel->geometry();
     positionBtnEdit = ui->pBtn_EditSave->geometry();

     positionEditInputName_t = positionEditInputName.adjusted(-25,20,-25,20);
     positionLabelInputName_t = positionLabelInputName.adjusted(-25,20,-25,20);
     positionBtnCancel_t = positionBtnCancel.adjusted(-20,0,-20,0);
     positionBtnEdit_t = positionBtnEdit.adjusted(-20,0,-20,0);

     ui->tabWidget->setCurrentIndex(0);
     RefreshContent(0,1);
     QWidget * TabWidgetSelected = ui->tabWidget->widget(0);
     ui->label_Telenumber->setParent(TabWidgetSelected);
     ui->label_Telenumber->show();

     //connect(ui->pBtn_Edit,SIGNAL(clicked()),this,SLOT(on_pBtnEdit_Add_clicked()));


     mSystemTrayIcon = new QSystemTrayIcon(this);
     QIcon icon("tele.ico");

      mSystemTrayIcon->setIcon(icon);

      connect(mSystemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ReShowFromTray(QSystemTrayIcon::ActivationReason)));

      ThreadAdding.start();

      ThreadSearching = new CDynamicSelectThread(this);
      ThreadSearching->start();

      m_Modem = CModemPoolSerialPort::getInstance();


      connect(ui->pBtnShortMessage,SIGNAL(clicked()),this,SLOT(launchShorMessageForm()));

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

    m_Modem->closeAll();
    delete m_Modem;

    portsInfo.clear();

    //delete ThreadSearching;
    delete ui;

}

//关闭到托盘---------
void MainWindow::closeEvent(QCloseEvent *e)
{
//    e->ignore();
//    this->hide();
    ThreadAdding.stop();
    ThreadSearching->stop();
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
    QString strDisplay = QString::number(index) + " is selected";

    ui->label_indications->setText(strDisplay);

    QWidget * TabWidgetSelected = ui->tabWidget->widget(index);

    ui->label_Telenumber->setParent(TabWidgetSelected);

    ui->labelInputName->setParent(TabWidgetSelected);

    ui->lineEdit_InputName->setParent(TabWidgetSelected);

    ui->pBtn_EditSave->setParent(TabWidgetSelected);

    ui->pBtnCancel->setParent(TabWidgetSelected);

    ui->label_Telenumber->show();
    ui->label_Telenumber->setText("");

    ui->labelInputName->hide();
    ui->lineEdit_InputName->hide();
    ui->pBtn_EditSave->hide();
    ui->pBtnCancel->hide();

    ui->pBtnEdit_Add->show();
    ui->pBtnDail->show();
    ui->pBtnEdit_Add->setEnabled(false);

    ui->pBtn_DeleteContactor->setEnabled(false);
    ui->pBtn_Edit->setEnabled(false);

    ui->treeWidget->setEnabled(true);
    ui->pBtn_Edit->show();
    ui->pBtn_Dailout->show();
    ui->listWidget->setEnabled(true);

    ui->label_InputTeleNumber->hide();
    ui->lineEdit_InputTeleNumber->hide();
    ui->pBtn_AddContactor->show();
    ui->pBtn_DeleteContactor->show();

    ui->lineEdit_InputName->setEnabled(true);
    ui->lineEdit_InputTeleNumber->setEnabled(true);



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
               RefreshContent(0,1);
          }

          ui->lineEdit_InputName->setGeometry(positionEditInputName_t);
          ui->labelInputName->setGeometry(positionLabelInputName_t);
          ui->pBtnCancel->setGeometry(positionBtnCancel_t);
          ui->pBtn_EditSave->setGeometry(positionBtnEdit_t);

    }
        break;
    case 1:
        {

           if(NeedRead_CommRecordInfoAll == true)
           {

               m_CommRecordTopListAll = m_CCommRecordTable->getListTop();
               NeedRead_CommRecordInfoAll = false;
           }
           if(NeedDisplay_CommRecordInfoAll)
           {

               RefreshContent(1,1);
           }

           ui->lineEdit_InputName->setGeometry(positionEditInputName);
           ui->labelInputName->setGeometry(positionLabelInputName);
           ui->pBtnCancel->setGeometry(positionBtnCancel);
           ui->pBtn_EditSave->setGeometry(positionBtnEdit);

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
    CommRecordInfo recordToAdd;
    recordToAdd.startTime = QDateTime::currentDateTime();
    recordToAdd.telenum = "12612812901";
    recordToAdd.isCallConnected = true;
    recordToAdd.isCallIn = true;
    recordToAdd.callDuration = 3600;
    recordToAdd.ringTimes = 0;

   Operation_Result ret = m_CCommRecordTable->DeleteRecordsByTelenumber(recordToAdd);

   qDebug()<<"operation result is " + QString::number(ret);
   NeedRead_CommRecordInfoAll = true;

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

QSerialPort * curentPort;

void MainWindow::on_pBtnDail_clicked()
{
    // m_CChinesePinyinTable->initTable();
   /* ContactorInfo recordToUpdate;
    recordToUpdate.name = "ty张三";
    recordToUpdate.telenum = "12612812911";
    m_ContactorTable->InsertPinyinForRecord(recordToUpdate);*/
    ui->tabWidget->setCurrentIndex(2);

    QWidget * TabWidgetSelected = ui->tabWidget->widget(2);

        QComboBox * cbComs = new QComboBox(TabWidgetSelected);

        int num_ports = portsInfo.count();

        for(int i=0;i<num_ports;i++)
        {
           cbComs->addItem(portsInfo.at(i).portName());
        }

        connect(cbComs,SIGNAL(currentIndexChanged(int)),this,SLOT(portsChanged(int)));

        m_Modem->setPort(portsInfo.at(0));
        if(m_Modem->open(QIODevice::ReadWrite))
        {
            qDebug()<<"open comm port sucessfully";
        }

        cbComs->show();

        pbtn_OpenClose = new QPushButton(TabWidgetSelected);

        pbtn_OpenClose->setText("close");


        pbtn_OpenClose->show();

        QRect posCombo = cbComs->geometry();

        QRect posBtn = pbtn_OpenClose->geometry();

        posBtn.adjust(posCombo.width(),posCombo.height(),posCombo.width(),posCombo.height());

        pbtn_OpenClose->setGeometry(posBtn);
        connect(pbtn_OpenClose,SIGNAL(clicked()),this,SLOT(OpenClosePort()));

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
    ui->pBtnEdit_Add->setEnabled(false);

    ui->label_Telenumber->setText("");

    m_CCommRecordTable = new CCommRecordTable();
    //m_CommRecordInfoList = m_CCommRecordTable->getListAllFromDatabase();
    m_CommRecordTopListAll = m_CCommRecordTable->getListTop();
    NeedRead_CommRecordInfoAll = false;
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

    ui->pBtn_Edit->hide();
    ui->pBtn_Dailout->hide();
    ui->listWidget->setEnabled(false);

    m_ContactorTable->setOperation(AddContactor);

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
    ui->pBtnEdit_Add->setEnabled(false);

    ui->treeWidget->setEnabled(true);
    ui->pBtn_Edit->show();
    ui->pBtn_Dailout->show();
    ui->listWidget->setEnabled(true);

    ui->label_InputTeleNumber->hide();
    ui->lineEdit_InputTeleNumber->hide();
    ui->pBtn_AddContactor->show();
    ui->pBtn_DeleteContactor->show();

    ui->lineEdit_InputName->setEnabled(true);
    ui->lineEdit_InputTeleNumber->setEnabled(true);

    isAddingContactor = false;
}

void MainWindow::on_pBtn_EditSave_clicked()
{
    ContactorTableOperation op = m_ContactorTable->getOperation();
    ContactorInfo infoToOperate;
    Operation_Result opResult;

    int indexTab = ui->tabWidget->currentIndex();

    switch(op)
    {
    case AddContactor:
    {
        /*save data to database here*/

        infoToOperate.name = ui->lineEdit_InputName->text().trimmed();
        if(indexTab==1)
        {
           infoToOperate.telenum = ui->label_Telenumber->text().trimmed();
        }
        else if(indexTab==0)
        {
           infoToOperate.telenum = ui->lineEdit_InputTeleNumber->text().trimmed();
        }

        if(  infoToOperate.name.isEmpty()
           ||(infoToOperate.name==tr("请在此输入名字"))
                )
        {
            ui->lineEdit_InputName->setText("请在此输入名字");
        }
        else if(
                  indexTab==0
                &&(  infoToOperate.telenum.isEmpty()
                   ||(infoToOperate.telenum==tr("请在此输入号码"))
                  )
                )
        {
           ui->lineEdit_InputTeleNumber->setText("请在此输入号码");
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
           m_ContactorTable->setOperation(OperationFinished);
           opResult = m_ContactorTable->addOneRecord(infoToOperate);

            if( opResult == AddSuccess)
            {
                QString strResult = "添加成功";
                mutex.lock();
                ContactorlistToAdd.append(infoToOperate);
                mutex.unlock();
                ui->label_Telenumber->setText(strResult);

            }
            else
            {
                QString strResult = "adding result is " + QString::number(opResult);
                ui->label_Telenumber->setText(strResult);
            }
        }
        /*then display the adding result*/

    }
        break;
    case DeleteContacor:
    {
        infoToOperate.name = ui->lineEdit_InputName->text().trimmed();

        infoToOperate.telenum = ui->lineEdit_InputTeleNumber->text().trimmed();


           ui->pBtn_EditSave->setText("确定");

           QRect positionBtn = ui->pBtn_EditSave->geometry();

           // positionBtn.moveRight(2);
            //positionBtn.width();

           positionBtn.adjust(20,0,20,0);
           ui->pBtn_EditSave->setGeometry(positionBtn);
           ui->pBtnCancel->hide();
           m_ContactorTable->setOperation(OperationFinished);
           opResult = m_ContactorTable->DeleteOneRecord(infoToOperate);

            if( opResult == DeleteSuccess)
            {
                QString strResult = "删除成功";

                ui->label_Telenumber->setText(strResult);

            }
            else
            {
                QString strResult = "result is " + QString::number(opResult);
                ui->label_Telenumber->setText(strResult);
            }



        /*then display the adding result*/
    }
        break;
    case ModifyContactor:
    {
        infoToOperate.name = ui->lineEdit_InputName->text().trimmed();

        infoToOperate.telenum = ui->lineEdit_InputTeleNumber->text().trimmed();


        if(  infoToOperate.name.isEmpty()
           ||(infoToOperate.name==tr("请在此输入名字"))
                )
        {
            ui->lineEdit_InputName->setText("请在此输入名字");
        }
        else if(   infoToOperate.telenum.isEmpty()
                 ||(infoToOperate.telenum==tr("请在此输入号码"))
                )
        {
           ui->lineEdit_InputTeleNumber->setText("请在此输入号码");
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
           m_ContactorTable->setOperation(OperationFinished);
           opResult = m_ContactorTable->UpdateOneRecord(infoToOperate,conInfoSelected);

            if( opResult == UpdateSuccess)
            {
                QString strResult = "修改成功";
                mutex.lock();
                ContactorlistToAdd.append(infoToOperate);
                mutex.unlock();
                ui->label_Telenumber->setText(strResult);

            }
            else
            {
                QString strResult = "result is " + QString::number(opResult);
                ui->label_Telenumber->setText(strResult);
            }
        }
        /*then display the adding result*/
    }
        break;
    case OperationFinished:
    {
        ui->pBtn_EditSave->setText("添加");
        ui->labelInputName->hide();
        ui->lineEdit_InputName->hide();
        ui->pBtn_EditSave->hide();
        ui->pBtnCancel->hide();

        QRect positionBtn = ui->pBtn_EditSave->geometry();

        positionBtn.adjust(-20,0,-20,0);
        ui->pBtn_EditSave->setGeometry(positionBtn);

        ui->pBtnEdit_Add->show();
        ui->pBtnEdit_Add->setEnabled(false);
        ui->pBtnDail->show();
        ui->treeWidget->setEnabled(true);

        ui->pBtn_Edit->show();
        ui->pBtn_Edit->setEnabled(false);
        ui->pBtn_Dailout->show();
        ui->listWidget->setEnabled(true);

        ui->pBtn_DeleteContactor->show();
        ui->pBtn_DeleteContactor->setEnabled(false);

        ui->label_InputTeleNumber->hide();
        ui->lineEdit_InputTeleNumber->hide();

        ui->pBtn_AddContactor->show();

        ui->label_Telenumber->setText("");

        NeedDisplay_CommRecordInfoAll = true;
        m_ContactorInfoList = m_ContactorTable->getListAllFromDatabase();
        NeedDisplay_ContactorsInfoAll = true;

        ui->lineEdit_InputName->setEnabled(true);
        ui->lineEdit_InputTeleNumber->setEnabled(true);

      /*  m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(updateRecord()));
        m_timer->start(500);
        */
        //QTimer::singleShot(100, this, SLOT(updateRecord()));

        RefreshContent(0,1);
        RefreshContent(1,1);
    }
        break;
    default:
        break;

    }


    /*if(isAddingContactor)
    {


    }
    else
    {



       }*/
}

void MainWindow::updateRecord()
{
    ui->tabWidget->setCurrentIndex(1);
}


void MainWindow::RefreshContent(int index,bool displayAll)
{
    ContactorInfoList tmpContactorList;
    CommRecordTopList tmpCommTopList;

    if(displayAll)
    {
        tmpContactorList = m_ContactorInfoList;
        tmpCommTopList = m_CommRecordTopListAll;

    }
    else
    {
        tmpContactorList = m_ContactorFreshList;
        tmpCommTopList = m_topFreshCommRecordList;
    }

    if(index==0)
    {
        /*refresh the contactors*/
        qDebug()<<"MainWindow::RefreshContent";
        ui->listWidget->clear();
        int num_ToAdd = tmpContactorList.count();

        QListWidgetItem * itemToAdd;

        for(int i=0;i<num_ToAdd;i++)
        {
             ContactorInfo oneRecord = tmpContactorList.at(i);
             QString str_ToAdd = oneRecord.name + " " + oneRecord.telenum;
             itemToAdd = new QListWidgetItem(str_ToAdd);

             //setting the value of the item.
             QVariant valueToSet;
             valueToSet.setValue(oneRecord);
             itemToAdd->setData(Qt::UserRole,valueToSet);

              ui->listWidget->insertItem(0,itemToAdd);
        }
        //set the flag after adding all items.
        if(displayAll)
        {
           NeedDisplay_ContactorsInfoAll=false;
        }
        else
        {
           NeedDisplay_ContactorsInfoAll=true;
        }

    }
    else if(index==1)
    {
        /*refresh the communication records*/
        qDebug()<<"MainWindow::RefreshContent comm";
        ui->treeWidget->clear();
        int num_ToAdd = tmpCommTopList.count();
        qDebug()<<"record number in comm record table: " + QString::number(num_ToAdd);
        QString str_sql_begin = "select * from communicate_record where telenumber = \'";
        QString str_sql_end = "\' order by startTime DESC";
        QString str_sql;

        int j=0;
        int numOneTeleNum;//number of records for one telephone number
        QTreeWidgetItem * ItemToAdd;
        CommRecordInfo oneFullRecord;
        CommRecordInfoList tempListOneNum;
        for(int i=0;i<num_ToAdd;i++)
        {//this loop adding list from communication record table
             CommRecordTopInfo oneTopRecord = tmpCommTopList.at(i);
             str_sql = str_sql_begin + oneTopRecord.telenum + str_sql_end;
             tempListOneNum = m_CCommRecordTable->getListBySql(str_sql);
             numOneTeleNum = tempListOneNum.count();

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

        if(!displayAll)
        {
           num_ToAdd = tmpContactorList.count();
           qDebug()<<"record number in contactor table: " + QString::number(num_ToAdd);
           for(int i=0;i<num_ToAdd;i++)
           {//this loop adding dynamic select for communication record.

               int index = m_CCommRecordTable->isTeleNumExistInTopList(tmpContactorList.at(i).telenum,tmpCommTopList);
               if(index!=-1)
               {
                  // m_CommRecordTree->topLevelItem(index);
                  qDebug()<<"record of contactor exist in comm record";
               }
               else
               {
                  str_sql = str_sql_begin + tmpContactorList.at(i).telenum + str_sql_end;
                  tempListOneNum = m_CCommRecordTable->getListBySql(str_sql);
                  numOneTeleNum = tempListOneNum.count();

                  for(j=0;j<numOneTeleNum;j++)
                {
                    oneFullRecord = tempListOneNum.at(j);

                    if(j==0)
                    {

                        QString strName = "";

                        telenumInfo infoToAdd;

                        infoToAdd.telenum = oneFullRecord.telenum;


                        strName = tmpContactorList.at(i).name;
                        infoToAdd.existInContactorTable = true;


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
           }
        }

        if(displayAll)
        {
           NeedDisplay_CommRecordInfoAll = false;
        }
        else
        {
           NeedDisplay_CommRecordInfoAll = true;
        }
    }
}

void MainWindow::setFreshList(CommRecordTopList commRecordList)
{
    m_topFreshCommRecordList.clear();
    m_topFreshCommRecordList = commRecordList;
}

void MainWindow::setFreshList(ContactorInfoList contactorList)
{
    qDebug()<<"in MainWindow::setFreshList ContactorInfoList";
    m_ContactorFreshList.clear();
    m_ContactorFreshList = contactorList;
}

void MainWindow::on_lineEdit_2_textChanged(const QString &arg1)
{

     int indexTabWidget = ui->tabWidget->currentIndex();

     if(!arg1.isEmpty())
     {
         mutex.lock();
         KeyWordToSelect.append(QString::number(indexTabWidget) + arg1);
         mutex.unlock();
     }
     else
     {
         RefreshContent(indexTabWidget,1);
     }

     ui->label_Telenumber->setText("");
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ContactorInfo infoClicked;

    infoClicked = item->data(Qt::UserRole).value<ContactorInfo>();

    ui->label_Telenumber->setText(infoClicked.telenum);

    ui->pBtn_Edit->setEnabled(true);
    ui->pBtn_DeleteContactor->setEnabled(true);

    conInfoSelected = infoClicked;

}

void MainWindow::on_pBtn_AddContactor_clicked()
{
    m_ContactorTable->setOperation(AddContactor);
    ui->labelInputName->show();
    ui->lineEdit_InputName->show();
    ui->pBtn_EditSave->show();
    ui->pBtnCancel->show();

    ui->pBtn_EditSave->setText("添加");

    ui->pBtnEdit_Add->hide();
    ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    ui->pBtn_Dailout->hide();
    ui->listWidget->setEnabled(false);


    ui->pBtn_AddContactor->hide();
    ui->pBtn_DeleteContactor->hide();
    ui->label_InputTeleNumber->show();
    ui->lineEdit_InputTeleNumber->show();
}

void MainWindow::on_pBtn_Edit_clicked()
{
    m_ContactorTable->setOperation(ModifyContactor);
    ui->labelInputName->show();
    ui->lineEdit_InputName->show();
    ui->lineEdit_InputName->setText(conInfoSelected.name);
    ui->pBtn_EditSave->show();
    ui->pBtnCancel->show();

    ui->pBtn_EditSave->setText("保存");

    ui->pBtnEdit_Add->hide();
    ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    ui->pBtn_Dailout->hide();
    ui->listWidget->setEnabled(false);


    ui->pBtn_AddContactor->hide();
    ui->pBtn_DeleteContactor->hide();
    ui->label_InputTeleNumber->show();
    ui->lineEdit_InputTeleNumber->setText(conInfoSelected.telenum);
    ui->lineEdit_InputTeleNumber->show();

    ui->label_Telenumber->setText("");
}



void MainWindow::on_pBtn_DeleteContactor_clicked()
{
    m_ContactorTable->setOperation(DeleteContacor);
    ui->labelInputName->hide();
   // ui->labelInputName->setText(conInfoSelected.name);
    ui->lineEdit_InputName->show();
    ui->lineEdit_InputName->setText(conInfoSelected.name);
    ui->lineEdit_InputName->setEnabled(false);

    ui->pBtn_EditSave->show();
    ui->pBtnCancel->show();

    ui->pBtn_EditSave->setText("删除");

    ui->pBtnEdit_Add->hide();
    ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    ui->pBtn_Dailout->hide();
    ui->listWidget->setEnabled(false);


    ui->pBtn_AddContactor->hide();
    ui->pBtn_DeleteContactor->hide();
    ui->label_InputTeleNumber->hide();
  //  ui->label_InputTeleNumber->setText(conInfoSelected.telenum);
    ui->lineEdit_InputTeleNumber->show();
    ui->lineEdit_InputTeleNumber->setText(conInfoSelected.telenum);
    ui->lineEdit_InputTeleNumber->setEnabled(false);



    ui->label_Telenumber->setText("");
}

void MainWindow::launchShorMessageForm()
{
    qDebug()<<"short message clicked";
    QDialog shortMessage;

    shortMessage.setWindowTitle("John 1321341589");
    shortMessage.setWindowIcon(QIcon(QPixmap("message.ico")));

    QPalette palette;
  //  palette.setColor(QPalette::Background, QColor(255,255,255));

    shortMessage.setAutoFillBackground(true);
    shortMessage.setPalette(palette);

    shortMessage.setBaseSize(600,800);
    QRect rectWindow(0,0,600,800);
    shortMessage.setFixedSize(600,800);


    QScrollArea * areaDisplayMessage = new QScrollArea(shortMessage.window());
  // palette.setColor(QPalette::Background, QColor(255,255,255));
   // areaDisplayMessage->setPalette(palette);

    areaDisplayMessage->setBaseSize(600,600);
    rectWindow = QRect(50,0,500,600);
    areaDisplayMessage->setGeometry(rectWindow);



    QWidget * wgScroll = new QWidget(shortMessage.window());
   // wgScroll->setMinimumWidth(800);
    wgScroll->setMinimumHeight(1000);
    wgScroll->setAutoFillBackground(true);
    palette.setColor(QPalette::Background, QColor(255,255,0));
    wgScroll->setPalette(palette);


        QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

    QFont font;
    font.setPointSize(14);

    //QLabel * text = new QLabel(shortMessage.window());
    QLabel * text = new QLabel(wgScroll);
    palette.setColor(QPalette::Background, QColor(0xcc,0xff,0x99));
    text->setAlignment(Qt::AlignLeft);
    text->setAutoFillBackground(true);
    text->setPalette(palette);
    text->setMaximumWidth(200);
    text->setFont(font);
    text->setWordWrap(true);

    text->setText("2015-5-10 12:13:02 \n 那天我去找你，你在哪儿？找你有急事,我看到你老爹，没有看到你");
    text->adjustSize();
    text->show();

    QRect rect_text = text->geometry();

    int wid = rect_text.width();
    int hei = rect_text.height();




    //int wid = rect_text.width();
    //int hei = rect_text.height();

    QLabel * text1 = new QLabel(wgScroll);
    palette.setColor(QPalette::Background, QColor(0xcc,0xdd,0xff));
    text1->setAlignment(Qt::AlignLeft);
    text1->setAutoFillBackground(true);
    text1->setPalette(palette);
    text1->setMaximumWidth(200);
    text1->setText("我在家里");
    rect_text = text1->geometry();
    rect_text.adjust(wid,hei,wid,hei);
    text1->setGeometry(rect_text);
    text1->setFont(font);
    text1->adjustSize();
    text1->show();

  //  QPushButton * pbtnTest = new QPushButton();
  //  pbtnTest->show();
    layout->addWidget(text);
    layout->addWidget(text1);

    wgScroll->setLayout(layout);

    //wgScroll->show();
    //areaDisplayMessage->show();


    QWidget *window = new QWidget(shortMessage.window());
        QPushButton *button1 = new QPushButton("One");
        QPushButton *button2 = new QPushButton("Two");
        QPushButton *button3 = new QPushButton("Three");
        QPushButton *button4 = new QPushButton("Four");
        QPushButton *button5 = new QPushButton("Five");

    /* QVBoxLayout*  layout = new QVBoxLayout;
        layout->addWidget(button1);
        layout->addWidget(button2);
        layout->addWidget(button3);
        layout->addWidget(button4);
        layout->addWidget(button5);
*/
     //   window->setLayout(layout);

        areaDisplayMessage->setWidget(wgScroll);
       // areaDisplayMessage->setWidget(window);
        wgScroll->show();

    this->hide();
    shortMessage.show();
    shortMessage.exec();
  //  delete text;
  //  delete areaDisplayMessage;
    this->show();

}

void MainWindow::portsChanged(int index)
{
    if(m_Modem->isOpen())
    {
        m_Modem->close();
        qDebug()<<"com port closed";
    }
    m_Modem->setPort(portsInfo.at(index));
    pbtn_OpenClose->setText("open");
   // m_Modem->open(QIODevice::ReadWrite);
}


void MainWindow::OpenClosePort()
{
    if(pbtn_OpenClose->text()=="close")
    {
        pbtn_OpenClose->setText("open");
        m_Modem->close();
    }
    else
    {
        m_Modem->open(QIODevice::ReadWrite);
        pbtn_OpenClose->setText("close");
    }
}
