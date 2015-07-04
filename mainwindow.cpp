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
#include <QHBoxLayout>

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

int widthListWidegtCon;
int heightListWidgetCon;
int topListWidegtCon;

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
    this->setFixedSize(870,660);
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
   //  closeButton->setStyleSheet("QToolButton{background-color:transparent;}");

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

     widthListWidegtCon = ui->listWidget->geometry().width();
     heightListWidgetCon = ui->listWidget->geometry().height();
     topListWidegtCon = ui->listWidget->geometry().top();
     ui->tabWidget->setCurrentIndex(0);
     RefreshContent(0,1);
     QWidget * TabWidgetSelected = ui->tabWidget->widget(0);
     ui->label_Telenumber->setParent(TabWidgetSelected);
     ui->label_Telenumber->show();

    // ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
     //connect(ui->pBtn_Edit,SIGNAL(clicked()),this,SLOT(on_pBtnEdit_Add_clicked()));


     mSystemTrayIcon = new QSystemTrayIcon(this);
     QIcon icon("tele.ico");

      mSystemTrayIcon->setIcon(icon);

      connect(mSystemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ReShowFromTray(QSystemTrayIcon::ActivationReason)));

      ThreadAdding.start();

      ThreadSearching = new CDynamicSelectThread(this);
      ThreadSearching->start();

      m_Modem = CModemPool::getInstance();

      if(m_CCommRecordTable!=NULL)
      {
          m_Modem->setCommRecordTable(m_CCommRecordTable);
      }
      connect(ui->pBtnShortMessage,SIGNAL(clicked()),this,SLOT(launchShorMessageForm()));
      connect(ui->pBtnDail,SIGNAL(clicked()),m_Modem,SLOT(preparePorts()));

      ui->pBtnDail->setParent(this->window());

      ui->pBtnDail->setGeometry(336,40,135,40);
      ui->pBtnDail->show();
      QRect posRelative = ui->pBtnDail->geometry();
      ui->pBtnMessageRecord->setGeometry(posRelative.right()+4,posRelative.top(),131,40);

      posRelative = ui->pBtnMessageRecord->geometry();
      ui->pBtnSendMessage->setGeometry(posRelative.right()+4,posRelative.top(),131,40);

     // m_Modem->setPushButton(ui->pBtnDail);
      adjustPosition();
      m_Modem->start();

      m_ShortMessageTable = NULL;
      NeedRead_ShortMessageRecordInfoAll = true;
      NeedDisplay_ShortMessageRecordInfoAll = true;
      m_messageTopInfoList.clear();

     // QString str = QString().fromUcs4("0891683110802105F0240D91683145117335F90008517040801572230C4F60898153BB54EA513FFF1F​");
     //QString str =  QString("0891683110802105F0240D91683145117335F90008517040801572230C4F60898153BB54EA513FFF1F​");
     QString str =  QString("4F60597D");
     QString strToDisplay = CShortMessageTable::usc4StringToNormalString(str);//codec->fromUnicode(str);//

    // QString str1 = QString("你好");
   //  QVector<uint> ucs4Vector = str1.toUcs4();

  //   QString strToDisplay1 = QString::fromUcs4(ucs4Vector.data());
     ui->label_indications->setText(strToDisplay);
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

   // ui->tabWidget->widget(2)->children()->clear();

    m_Modem->stop();
    m_Modem->closeAllPorts();
    //delete m_Modem;

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

    ui->lineEdit_2->setText("");

    QWidget * TabWidgetSelected = ui->tabWidget->widget(index);


    if((index==0)||(index==1)||(index==4))
    {
       ui->label_Telenumber->setParent(TabWidgetSelected);

       ui->labelInputName->setParent(TabWidgetSelected);

       ui->lineEdit_InputName->setParent(TabWidgetSelected);

       ui->pBtn_EditSave->setParent(TabWidgetSelected);

       ui->pBtnCancel->setParent(TabWidgetSelected);

      if(index==0)
      {
          ui->listWidget->setParent(TabWidgetSelected);
          QRect pos = ui->listWidget->geometry();
          pos.setWidth(widthListWidegtCon);
          pos.setHeight(heightListWidgetCon);
          pos.setTop(topListWidegtCon);
          ui->listWidget->setGeometry(pos.left(),topListWidegtCon,widthListWidegtCon,heightListWidgetCon);
          ui->listWidget->show();
          ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
          ui->listWidget->setCurrentRow(-1);
      }


    }
    else
    {
        if(index==2)
        {
           ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
           ui->listWidget->setParent(TabWidgetSelected);
           QRect pos = ui->listWidget->geometry();
           ui->listWidget->setGeometry(pos.left(),topListWidegtCon+40,widthListWidegtCon-120,heightListWidgetCon-110);
           ui->listWidget->show();
           ui->listWidget->setCurrentRow(-1);

           ui->pBtnEdit_Add->setParent(TabWidgetSelected);
           ui->pBtnEdit_Add->show();
        }

    }

    if((index==1)||(index==4))
    {
         ui->pBtnEdit_Add->setParent(TabWidgetSelected);
         ui->pBtnEdit_Add->show();
    }
    ui->label_Telenumber->show();
    ui->label_Telenumber->setText("");

    ui->labelInputName->hide();
    ui->lineEdit_InputName->hide();
    ui->pBtn_EditSave->hide();
    ui->pBtnCancel->hide();

    ui->pBtnEdit_Add->show();
  //  ui->pBtnDail->show();
    ui->pBtnEdit_Add->setEnabled(false);

    ui->pBtn_DeleteContactor->setEnabled(false);
    ui->pBtn_Edit->setEnabled(false);

    ui->treeWidget->setEnabled(true);
    ui->pBtn_Edit->show();
   // ui->pBtn_Dailout->show();
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

       /*   ui->lineEdit_InputName->setGeometry(positionEditInputName_t);
          ui->labelInputName->setGeometry(positionLabelInputName_t);
          ui->pBtnCancel->setGeometry(positionBtnCancel_t);
          ui->pBtn_EditSave->setGeometry(positionBtnEdit_t);*/

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

         /*  ui->lineEdit_InputName->setGeometry(positionEditInputName);
           ui->labelInputName->setGeometry(positionLabelInputName);
           ui->pBtnCancel->setGeometry(positionBtnCancel);
           ui->pBtn_EditSave->setGeometry(positionBtnEdit);*/

        }
        break;
    case 2:
    {
      //  QWidget * TabWidgetSelected = ui->tabWidget->widget(2);

      /*  QLabel * labelSelectSIM = new QLabel("请选择SIM进行拨号",TabWidgetSelected);
        labelSelectSIM->adjustSize();
        labelSelectSIM->show();

        QSize sizeLabel = labelSelectSIM->size();

        qDebug()<<"height is " << sizeLabel.height();
        qDebug()<<"width is" << sizeLabel.width();

        label_SIMNumber = new QLabel("SIM不可用",TabWidgetSelected);
        label_SIMNumber->adjustSize();
        label_SIMNumber->show();

        QComboBox * cbComs = new QComboBox(TabWidgetSelected);

            int num_ports = m_Modem->portsCount();

            for(int i=0;i<num_ports;i++)
            {
               cbComs->addItem(m_Modem->getPortInfo(i).portName());
            }

            connect(cbComs,SIGNAL(currentIndexChanged(int)),this,SLOT(portsChanged(int)));

            CModemPoolSerialPort * com1 = m_Modem->getSIMPort(0);
            if(com1!=NULL)
            {
               if(com1->open(QIODevice::ReadWrite))
               {
                   qDebug()<<"open comm port sucessfully";
               }
            }

            cbComs->show();

            pbtn_OpenClose = new QPushButton(TabWidgetSelected);

            pbtn_OpenClose->setText("close");

            pbtn_OpenClose->show();

            QRect posCombo = cbComs->geometry();

            QRect posBtn = pbtn_OpenClose->geometry();

            QRect posLbSIMSel = labelSelectSIM->geometry();

            QRect posLbSIMNum = label_SIMNumber->geometry();

           // posCombo.adjust(0,posLbSIMSel.height(),0,posLbSIMSel.height());
            posCombo.moveTop(posLbSIMSel.height());
            posCombo.adjust(0,5,posLbSIMSel.width()-posCombo.width(),5);
            cbComs->setGeometry(posCombo);

            posLbSIMNum.moveTopLeft(posCombo.topRight());
            posLbSIMNum.adjust(10,0,10,0);
            label_SIMNumber->setGeometry(posLbSIMNum);
           // label_SIMNumber->hide();

            posBtn.moveTop(posCombo.bottom()+5);
            pbtn_OpenClose->setGeometry(posBtn);


        connect(pbtn_OpenClose,SIGNAL(clicked()),this,SLOT(OpenClosePort()));*/

        QPushButton * pBtn = TabWidgetSelected->findChild<QPushButton  *>("pBtn_AddNumber");
        if(pBtn==0)
        {
           QPushButton  * pBtn_AddNumber = new QPushButton(TabWidgetSelected);
           pBtn_AddNumber->setObjectName("pBtn_AddNumber");
           QPixmap addPix  = style()->standardPixmap(QStyle::SP_ArrowRight);
           pBtn_AddNumber->setIcon(addPix);
           pBtn_AddNumber->setGeometry(widthListWidegtCon-118,heightListWidgetCon/4,20,20);
           pBtn_AddNumber->setToolTip(tr("添加"));
           pBtn_AddNumber->setStyleSheet("QPushButton{background-color:transparent;}"
                                    "QPushButton:hover{background-color:grey;}");
            pBtn_AddNumber->show();
            connect(pBtn_AddNumber,SIGNAL(clicked()),this,SLOT(AddNumsProcess()));
        }
        pBtn = TabWidgetSelected->findChild<QPushButton  *>("pBtn_DelNumber");
        if(pBtn==0)
        {
            QPushButton  * pBtn_DelNumber = new QPushButton(TabWidgetSelected);
            pBtn_DelNumber->setObjectName("pBtn_DelNumber");
            QPixmap delPix = style()->standardPixmap(QStyle::SP_ArrowLeft);

            pBtn_DelNumber->setIcon(delPix);

            pBtn_DelNumber->setGeometry(widthListWidegtCon-118,heightListWidgetCon/2,20,20);
            pBtn_DelNumber->setToolTip(tr("删除"));
            pBtn_DelNumber->setStyleSheet("QPushButton{background-color:transparent;}"
                                       "QPushButton:hover{background-color:grey;}");

            pBtn_DelNumber->show();

            connect(pBtn_DelNumber,SIGNAL(clicked()),this,SLOT(DelNumsProcess()));
        }
        QLabel *label = TabWidgetSelected->findChild<QLabel *>("label_contactors");

        if(label==0)
        {
           QLabel * label_contactors = new QLabel(TabWidgetSelected);
           label_contactors->setObjectName("label_contactors");
           label_contactors->setGeometry(0,topListWidegtCon,widthListWidegtCon-120,40);
           label_contactors->setText("联系人列表:");
           label_contactors->setAlignment(Qt::AlignCenter);
           label_contactors->show();
        }

        QTreeWidget * tree = TabWidgetSelected->findChild<QTreeWidget *>("treeWidgetNumsNeedProcess");
        if(tree==0)
        {
            QTreeWidget * treeWidgetNumsNeedProcess = new QTreeWidget(TabWidgetSelected);
            treeWidgetNumsNeedProcess->setGeometry(widthListWidegtCon-90,topListWidegtCon,widthListWidegtCon-90,heightListWidgetCon-70);
            treeWidgetNumsNeedProcess->setObjectName("treeWidgetNumsNeedProcess");
            QStringList headers;
            headers.clear();
            headers<<"已选号码"<<"状态";
            treeWidgetNumsNeedProcess->setColumnCount(2);
            treeWidgetNumsNeedProcess->setHeaderLabels(headers);
            treeWidgetNumsNeedProcess->setSelectionMode(QAbstractItemView::ExtendedSelection);
            treeWidgetNumsNeedProcess->show();

            m_Modem->setTreeWidget(treeWidgetNumsNeedProcess);
        }

        pBtn = TabWidgetSelected->findChild<QPushButton  *>("pBtnStart");
        if(pBtn==0)
        {
           QPushButton * pBtnStart = new QPushButton(TabWidgetSelected);
           pBtnStart->setObjectName("pBtnStart");
           pBtnStart->setText("开始");
           pBtnStart->setGeometry(widthListWidegtCon-110,heightListWidgetCon-60,50,30);
           pBtnStart->show();

           connect(pBtnStart,SIGNAL(clicked()),m_Modem,SLOT(startProcess()));
          // connect(pBtnStart,SIGNAL(clicked()),this,SLOT(endProcess()));
           connect(m_Modem,SIGNAL(endProcess()),this,SLOT(endProcess()));
           pBtnStart->setEnabled(false);
           m_Modem->setPushButton(pBtnStart);
        }

    }
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
   // ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    //ui->pBtn_Dailout->hide();
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
   // ui->pBtnDail->show();
    ui->pBtnEdit_Add->setEnabled(false);

    ui->treeWidget->setEnabled(true);
    ui->pBtn_Edit->show();
    ui->pBtn_Edit->setEnabled(false);
    //ui->pBtn_Dailout->show();
    ui->listWidget->setEnabled(true);

    ui->label_InputTeleNumber->hide();
    ui->lineEdit_InputTeleNumber->hide();
    ui->pBtn_AddContactor->show();
    ui->pBtn_DeleteContactor->show();
    ui->pBtn_DeleteContactor->setEnabled(false);

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
        if((indexTab==1)||(indexTab==4))
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

           //QRect positionBtn = ui->pBtn_EditSave->geometry();

           // positionBtn.moveRight(2);
            //positionBtn.width();

          // positionBtn.adjust(20,0,20,0);
         //  ui->pBtn_EditSave->setGeometry(positionBtn);
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
       // ui->pBtnDail->show();
        ui->treeWidget->setEnabled(true);

        ui->pBtn_Edit->show();
        ui->pBtn_Edit->setEnabled(false);
       // ui->pBtn_Dailout->show();
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
        //NeedDisplay_ShortMessageRecordInfoAll = true;
        refreshMessageDisplay(1);
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

    if((index==0)||(index==2))
    {
        /*refresh the contactors*/
        qDebug()<<"MainWindow::RefreshContent";
        ui->listWidget->clear();
        int num_ToAdd = tmpContactorList.count();

        QListWidgetItem * itemToAdd;

        for(int i=0;i<num_ToAdd;i++)
        {
             ContactorInfo oneRecord = tmpContactorList.at(i);
             if( (index!=2)
                ||( (index==2)
                  &&(numsNeedProcess.contains(oneRecord.telenum)==false)
                  )
              )
             {
                QString str_ToAdd = oneRecord.name + " " + oneRecord.telenum;
                itemToAdd = new QListWidgetItem(str_ToAdd);

                //setting the value of the item.
                QVariant valueToSet;
                valueToSet.setValue(oneRecord);
                itemToAdd->setData(Qt::UserRole,valueToSet);

                ui->listWidget->insertItem(0,itemToAdd);
             }
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

        if(!displayAll)//display the records found through input keyword from contactor table,
            //but not in reord comm table, this is to list all infomation found for record ui,seems
            //no need to do so now.
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
   // ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    //ui->pBtn_Dailout->hide();
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
   // ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    //ui->pBtn_Dailout->hide();
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
    //ui->pBtnDail->hide();
    ui->treeWidget->setEnabled(false);

    ui->pBtn_Edit->hide();
    //ui->pBtn_Dailout->hide();
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
    //rectWindow = QRect(50,0,500,600);
    wgScroll->setMaximumWidth(500);
 //   wgScroll->setMinimumHeight(1000);
    wgScroll->setAutoFillBackground(true);
    palette.setColor(QPalette::Background, QColor(255,255,255));
    wgScroll->setPalette(palette);


        QGridLayout *layout = new QGridLayout();

    QFont font;
    font.setPointSize(14);

    //QLabel * text = new QLabel(shortMessage.window());
    QLabel * text = new QLabel(wgScroll);
    palette.setColor(QPalette::Background, QColor(0xcc,0xff,0x99));
    text->setAlignment(Qt::AlignLeft);
    text->setAutoFillBackground(true);
    text->setPalette(palette);
    text->setMaximumWidth(350);
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
    text1->setMaximumWidth(350);
    text1->setText("我在家里oytytytythnncbvhfhnnbchskfsfhkshfkafhsdakjdfhasd");
    text1->setWordWrap(true);
    text1->setFont(font);
    text1->adjustSize();
    text1->show();

  //  QPushButton * pbtnTest = new QPushButton();
  //  pbtnTest->show();
  //  layout->addWidget(text,0,0);
  //  layout->addWidget(text1,1,0,Qt::AlignRight);

    QHBoxLayout *layoutline = new QHBoxLayout();
    layoutline->addWidget(text);
    layoutline->insertSpacing(1,150);
    layoutline->setMargin(0);
    layout->addLayout(layoutline,0,0);

    layoutline = new QHBoxLayout();
    layoutline->insertSpacing(0,150);
    layoutline->addWidget(text1);
    layoutline->setMargin(0);
    layout->addLayout(layoutline,1,0);

    layout->setMargin(0);

    rect_text = text1->geometry();
    rect_text.adjust(wid,hei,wid,hei);
    text1->setGeometry(rect_text);



    //wgScroll->show();
    //areaDisplayMessage->show();


  /*  QWidget *window = new QWidget(shortMessage.window());
        QPushButton *button1 = new QPushButton("One",window);
        QPushButton *button2 = new QPushButton("Two");
        QPushButton *button3 = new QPushButton("Three");
        QPushButton *button4 = new QPushButton("Four");
        QPushButton *button5 = new QPushButton("Five");

     QVBoxLayout*  layout = new QVBoxLayout;
        layout->addWidget(button1);
        layout->addWidget(button2);
        layout->addWidget(button3);
        layout->addWidget(button4);
        layout->addWidget(button5);
*/
     //   window->setLayout(layout);


       // layout->addWidget(window);
        wgScroll->setLayout(layout);
        areaDisplayMessage->setWidget(wgScroll);
       // areaDisplayMessage->setWidget(window);
        wgScroll->show();
     //   window->show();

    this->hide();
    shortMessage.show();
    shortMessage.exec();
  //  delete text;
  //  delete areaDisplayMessage;
    this->show();

}

void MainWindow::portsChanged(int index)
{
  /*  if(m_Modem->isOpen())
    {
        m_Modem->close();
        qDebug()<<"com port closed";
    }
    m_Modem->setPort(portsInfo.at(index));
*/
    CModemPoolSerialPort * PortSelected = m_Modem->getSIMPort(index);
    PortSelected->open(QIODevice::ReadWrite);
    if(PortSelected->isOpen())
    {
        qDebug()<<"comm port open";
        QByteArray ba = "AT\n";
        char * strToWrite = ba.data();
        PortSelected->write(strToWrite);

        ba = "AT+CPBS=\"ON\"\n";
        strToWrite = ba.data();
        qDebug()<<strToWrite;
        PortSelected->write(strToWrite);

        ba = "AT+CLIP=\"ON\"\n";
        strToWrite = ba.data();
        PortSelected->write(strToWrite);


       /* if(PortSelected->)
        strToWrite = "AT+CNUM\n";
        qDebug()<<strToWrite;
        */
        /*PortSelected->write(strToWrite);

        PortSelected->waitForReadyRead(50);

        QByteArray strRead = PortSelected->readAll();

        if(strRead.contains("OK"))//AT bout rate syncronized
        {
            strToWrite = "AT+CPBS=\"ON\"\n";
            qDebug()<<strToWrite;
            PortSelected->write(strToWrite);

            PortSelected->waitForReadyRead(50);
            strRead = PortSelected->readAll();
            if(strRead.contains("OK"))//open tele book
            {
               strToWrite = "AT+CNUM\n";
               qDebug()<<strToWrite;
               PortSelected->write(strToWrite);

               PortSelected->waitForReadyRead(50);
               strRead = PortSelected->readAll();
               PortSelected->waitForReadyRead(50);
               strRead = PortSelected->readAll();
            }
        }*/

        //qDebug()<<strRead;
    }
    //pbtn_OpenClose->setText("open");
   // m_Modem->open(QIODevice::ReadWrite);
}


void MainWindow::OpenClosePort()
{

    if(pbtn_OpenClose->text()=="close")
    {
        pbtn_OpenClose->setText("open");
       // m_Modem->close();
    }
    else
    {
       // m_Modem->open(QIODevice::ReadWrite);
        pbtn_OpenClose->setText("close");
    }
}

void MainWindow::AddNumsProcess()
{
    QTreeWidget * treeWidgetNumsNeedProcess = ui->tabWidget->widget(2)->findChild<QTreeWidget *>("treeWidgetNumsNeedProcess");

    QList<QListWidgetItem *> items = ui->listWidget->selectedItems();
    QTreeWidgetItem * itemToAdd;
    ContactorInfo oneRecord;
    QVariant dataInItem;
    int numsToAdd = items.count();
    int countBegin = numsNeedProcess.count();

    for(int i=0;i<numsToAdd;i++)
    {
        //and constraint here when numsNeedProcess contains the num to add.

       dataInItem = items.at(i)->data(Qt::UserRole);
       oneRecord = dataInItem.value<ContactorInfo>();
       if(numsNeedProcess.contains(oneRecord.telenum)==false)
       {
          itemToAdd = new QTreeWidgetItem();
          itemToAdd->setText(0,oneRecord.name + " " + oneRecord.telenum);
          itemToAdd->setText(1,"待处理");
          itemToAdd->setData(0,Qt::UserRole,dataInItem);
          treeWidgetNumsNeedProcess->insertTopLevelItem(i+countBegin,itemToAdd);
          mutex.lock();
          numsNeedProcess.append(oneRecord.telenum);
          mutex.unlock();
          ui->listWidget->removeItemWidget(items.at(i));
          delete items.at(i);
       }
    }

    treeWidgetNumsNeedProcess->show();
    ui->listWidget->show();

    QPushButton * pBtnStart;
    if(treeWidgetNumsNeedProcess->topLevelItemCount()!=0)
    {
        pBtnStart =  ui->tabWidget->widget(2)->findChild<QPushButton  *>("pBtnStart");
        pBtnStart->setEnabled(true);
    }
    else
    {
        pBtnStart =  ui->tabWidget->widget(2)->findChild<QPushButton  *>("pBtnStart");
        pBtnStart->setEnabled(false);
    }


}

void MainWindow::DelNumsProcess()
{
    QTreeWidget * treeWidgetNumsNeedProcess = ui->tabWidget->widget(2)->findChild<QTreeWidget *>("treeWidgetNumsNeedProcess");

    QList<QTreeWidgetItem*> items = treeWidgetNumsNeedProcess->selectedItems();


    QListWidgetItem * itemToAdd;
    ContactorInfo oneRecord;
    QVariant dataInItem;
    int numsToAdd = items.count();
    int countBegin = ui->listWidget->count();

    for(int i=0;i<numsToAdd;i++)
    {
        //and constraint here when numsNeedProcess contains the num to add.

       dataInItem = items.at(i)->data(0,Qt::UserRole);
       oneRecord = dataInItem.value<ContactorInfo>();
       if(numsNeedProcess.contains(oneRecord.telenum)==true)
       {
          itemToAdd = new QListWidgetItem();
          itemToAdd->setText(oneRecord.name + " " + oneRecord.telenum);
          itemToAdd->setData(Qt::UserRole,dataInItem);
          ui->listWidget->insertItem(i+countBegin,itemToAdd);
          mutex.lock();
          numsNeedProcess.removeOne(oneRecord.telenum);
          mutex.unlock();
          treeWidgetNumsNeedProcess->removeItemWidget(items.at(i),0);
          treeWidgetNumsNeedProcess->removeItemWidget(items.at(i),1);
          delete items.at(i);
       }
    }

    treeWidgetNumsNeedProcess->show();
    ui->listWidget->show();

    QPushButton * pBtnStart;
    if(treeWidgetNumsNeedProcess->topLevelItemCount()!=0)
    {
        pBtnStart = ui->tabWidget->widget(2)->findChild<QPushButton  *>("pBtnStart");
        pBtnStart->setEnabled(true);
    }
    else
    {
        pBtnStart =  ui->tabWidget->widget(2)->findChild<QPushButton  *>("pBtnStart");
        pBtnStart->setEnabled(false);
    }

}

void MainWindow::endProcess()
{


    QTreeWidget * treeWidgetNumsNeedProcess = ui->tabWidget->widget(2)->findChild<QTreeWidget *>("treeWidgetNumsNeedProcess");

    //QList<QTreeWidgetItem*> items = treeWidgetNumsNeedProcess->selectedItems();

    QTreeWidgetItem* topLevelItem;

    QListWidgetItem * itemToAdd;
    ContactorInfo oneRecord;
    QVariant dataInItem;
    int numsToAdd = treeWidgetNumsNeedProcess->topLevelItemCount();
    int countBegin = ui->listWidget->count();
    qDebug()<<"num of items in tree "<<numsToAdd;
    for(int i=0;i<numsToAdd;i++)
    {
        //and constraint here when numsNeedProcess contains the num to add.
       topLevelItem = treeWidgetNumsNeedProcess->takeTopLevelItem(0);
       dataInItem = topLevelItem->data(0,Qt::UserRole);
       oneRecord = dataInItem.value<ContactorInfo>();

          itemToAdd = new QListWidgetItem();
          itemToAdd->setText(oneRecord.name + " " + oneRecord.telenum);
          itemToAdd->setData(Qt::UserRole,dataInItem);
          ui->listWidget->insertItem(i+countBegin,itemToAdd);
          mutex.lock();
          numsNeedProcess.removeOne(oneRecord.telenum);
          mutex.unlock();
         // treeWidgetNumsNeedProcess->removeItemWidget(items.at(i),0);
         // treeWidgetNumsNeedProcess->removeItemWidget(items.at(i),1);
          //delete items.at(i);

       delete topLevelItem;

    }

    treeWidgetNumsNeedProcess->show();
    ui->listWidget->show();

    QPushButton * pBtnStart;

    pBtnStart =  ui->tabWidget->widget(2)->findChild<QPushButton  *>("pBtnStart");
    pBtnStart->setEnabled(false);


}


void MainWindow::adjustPosition()
{
    //this function should be called after widthListWidegtCon is set.
    //setting the position of tree widget for communication record.
    QRect geoTree = ui->treeWidget->geometry();
    geoTree.setWidth(widthListWidegtCon);
    //get the rect of pBtn_AddContactor, and this is used for relative standard.
    QRect posBegin = ui->pBtn_AddContactor->geometry();

    int wideOfStandard = posBegin.width();
    int heightOfStandard = posBegin.height();
    int bottomOfStandard = posBegin.bottom();
    int leftOfStandard = posBegin.left();
   // int topOfStandard = posBegin.top();

    //setting the position of label to display the number for both contactor and communication record.
    ui->label_Telenumber->setGeometry(leftOfStandard-10,bottomOfStandard+20,wideOfStandard+20,heightOfStandard);

    //setting the label and editline of input telenumber
    ui->label_InputTeleNumber->setGeometry(leftOfStandard-20,bottomOfStandard+40+heightOfStandard,wideOfStandard+40,heightOfStandard);
    QRect posNext = ui->label_InputTeleNumber->geometry();
    posNext.adjust(0,heightOfStandard+2,0,heightOfStandard+2);
    ui->lineEdit_InputTeleNumber->setGeometry(posNext);
    posNext = ui->lineEdit_InputTeleNumber->geometry();

    //setting the label and line edit for name
    bottomOfStandard = posNext.bottom();
    leftOfStandard = posNext.left();
    wideOfStandard = posNext.width();
    heightOfStandard = posNext.height();
    ui->labelInputName->setGeometry(leftOfStandard,bottomOfStandard+20,wideOfStandard,heightOfStandard);
    ui->lineEdit_InputName->setGeometry(leftOfStandard,bottomOfStandard+22+heightOfStandard,wideOfStandard,heightOfStandard);

    //setting the Add button and cancel button
    posNext = ui->lineEdit_InputName->geometry();
    bottomOfStandard = posNext.bottom();
    leftOfStandard = posNext.left();
    wideOfStandard = posNext.width();
    heightOfStandard = posNext.height();
    QRect posSave = ui->pBtn_EditSave->geometry();
    ui->pBtn_EditSave->setGeometry(leftOfStandard+7,bottomOfStandard+20,posSave.width(),posSave.height());
    posSave = ui->pBtn_EditSave->geometry();

    posNext = ui->pBtnCancel->geometry();
    ui->pBtnCancel->setGeometry(posSave.right()+20,posSave.top(),posNext.width(),posNext.height());
    posNext = ui->lineEdit_InputName->geometry();

    //setting pushsbutton add to contactor in comm record interface.
    posSave = ui->pBtnEdit_Add->geometry();
    ui->pBtnEdit_Add->setGeometry(posNext.left()+12,posNext.top(),posSave.width(),posSave.height());

    //hide the dial button on the contactor interface.
    ui->pBtn_Dailout->hide();

}

void MainWindow::on_pBtnMessageRecord_clicked()
{
    ui->tabWidget->setCurrentIndex(4);
    QWidget* tab4 = ui->tabWidget->widget(4);

    QTreeWidget * treeWidgetMessageRecord = tab4->findChild<QTreeWidget *>("treeWidgetMessageRecord");
    if(treeWidgetMessageRecord==0)
    {
        QTreeWidget * treeWidgetMessageRecord = new QTreeWidget(tab4);
        treeWidgetMessageRecord->setGeometry(0,topListWidegtCon,widthListWidegtCon,heightListWidgetCon);
        treeWidgetMessageRecord->setObjectName("treeWidgetMessageRecord");
        QStringList headers;
        headers.clear();
        headers<<"号码/联系人"<<"状态";
        treeWidgetMessageRecord->setColumnCount(2);
        treeWidgetMessageRecord->setHeaderLabels(headers);
       // treeWidgetNumsNeedProcess->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeWidgetMessageRecord->show();
        treeWidgetMessageRecord->setColumnWidth(0,300);

        connect(treeWidgetMessageRecord,SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(shortMessageTree_ItemClicked(QTreeWidgetItem*, int)));

       // m_Modem->setTreeWidget(treeWidgetMessageRecord);
    }



    //read from short message table and display.
    if(m_ShortMessageTable==NULL)
    {
        m_ShortMessageTable = new CShortMessageTable();
    }
    if(NeedRead_ShortMessageRecordInfoAll)
    {
        m_messageTopInfoList = m_ShortMessageTable->getListTop();
    }
    if(NeedDisplay_ShortMessageRecordInfoAll)
    {
       refreshMessageDisplay(1);
    }
    else
    {
       refreshMessageDisplay(0);
    }

}

void MainWindow::refreshMessageDisplay(bool displayAll)
{
    messageTopInfoList tmpMessageTopInfoList;

    if(displayAll)
    {
        tmpMessageTopInfoList = m_messageTopInfoList;
    }
    else
    {
        tmpMessageTopInfoList = m_messageFreshTopInfoList;
    }

    QWidget* tab4 = ui->tabWidget->widget(4);

    QTreeWidget * treeWidgetMessageRecord = tab4->findChild<QTreeWidget *>("treeWidgetMessageRecord");

    if((tmpMessageTopInfoList.count()!=0)&&(treeWidgetMessageRecord!=NULL))
    {
    /*refresh the communication records*/
    qDebug()<<"messageRecord::RefreshContent";
    treeWidgetMessageRecord->clear();
    int num_ToAdd = tmpMessageTopInfoList.count();
    qDebug()<<"record number to display: " + QString::number(num_ToAdd);
    QString str_sql_begin = "select * from shortMessage where NumberRemote = \'";
    QString str_sql_end = "\' order by send_recvTime DESC";
    QString str_sql;

    int j=0;
    int numOneTeleNum;//number of records for one telephone number
    QTreeWidgetItem * ItemToAdd;
    messageInfo oneFullRecord;
    messageInfoList tempListOneNum;
    for(int i=0;i<num_ToAdd;i++)
    {//this loop adding list from shortMessage table
         messageTopInfo oneTopRecord = tmpMessageTopInfoList.at(i);
         str_sql = str_sql_begin + oneTopRecord.telenum + str_sql_end;
         tempListOneNum = m_ShortMessageTable->getListBySql(str_sql);
         numOneTeleNum = tempListOneNum.count();

         for(j=0;j<numOneTeleNum;j++)
         {
             oneFullRecord = tempListOneNum.at(j);

             if(j==0)
             {
                 QString str_SelectContactor = "select * from contactors where telenumber = \'" + oneFullRecord.NumberRemote + "\'";

                 ContactorInfoList tempContactor =  m_ContactorTable->getListBySql(str_SelectContactor);

                 QString strName = "";

                 telenumInfo infoToAdd;

                 infoToAdd.telenum = oneFullRecord.NumberRemote;


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

                 QStringList strList = QStringList()<<oneFullRecord.NumberRemote+" "+strName;
                 ItemToAdd = new QTreeWidgetItem(strList);
                 ItemToAdd->setData(0, Qt::UserRole,	valueToAdd);
                 treeWidgetMessageRecord->addTopLevelItem(ItemToAdd);

                 QString str_record = m_ShortMessageTable->ConstructRecordString(oneFullRecord);


                 strList = QStringList()<<str_record;

                 if(oneFullRecord.isReceived)
                 {
                    if(oneFullRecord.readed)
                    {
                       strList.append("已读");
                    }
                    else
                    {
                       strList.append("未读");
                    }
                 }
                 else
                 {
                     strList.append("已发送");
                 }

                 QTreeWidgetItem * childItemToAdd = new QTreeWidgetItem(strList);

                 childItemToAdd->setToolTip(0,str_record);

                 ItemToAdd->addChild(childItemToAdd);
             }
             else
             {
                 QString str_record = m_ShortMessageTable->ConstructRecordString(oneFullRecord);
                 QStringList strList = QStringList()<<str_record;
                 if(oneFullRecord.isReceived)
                 {
                    if(oneFullRecord.readed)
                    {
                       strList.append("已读");
                    }
                    else
                    {
                       strList.append("未读");
                    }
                 }
                 else
                 {
                     strList.append("已发送");
                 }
                 QTreeWidgetItem * childItemToAdd = new QTreeWidgetItem(strList);

                 childItemToAdd->setToolTip(0,str_record);
                 ItemToAdd->addChild(childItemToAdd);
             }
         }
    }
     if(NeedDisplay_ShortMessageRecordInfoAll)
     {
        NeedDisplay_ShortMessageRecordInfoAll = false;
     }
     else
     {
        NeedDisplay_ShortMessageRecordInfoAll = true;
     }
    }

}

void MainWindow::shortMessageTree_ItemClicked(QTreeWidgetItem *item, int column)
{
    m_treeItemActive = item;
    QString strDisplay;// = QString::number(column) + " clicked";


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
