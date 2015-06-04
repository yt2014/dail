#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QTextCodec>
#include <QVBoxLayout>

#include "QPushButtonExit.h"

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

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowOpacity(1);

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);
    //QString  textSet = QString::fromUtf8("联系人");

  //  QString Str_Set = codec->convertToUnicode(textSet);

    QToolButton *tBtn_Contractors = new QToolButton;
    tBtn_Contractors->setText(QStringLiteral("联系人"));
    QToolButton *tBtn_Dail = new QToolButton;
    tBtn_Dail->setText(QStringLiteral("拨  号"));

    ui->mainToolBar->addWidget(tBtn_Contractors);
    ui->mainToolBar->addWidget(tBtn_Dail);

    ui->mainToolBar->hide();
    this->ui->pBtn_Contactors->setFlat(1);
    this->ui->pBtn_Contactors->setMouseTracking(true);
    this->ui->pBtn_Dail->setFlat(1);

    this->ui->pBtn_Contactors->hide();
    this->ui->pBtn_Dail->hide();

    QPushButtonExit * BtnExit = new QPushButtonExit();
   // this->ui->mainToolBar->addWidget(BtnExit);
   BtnExit->setText("eixt test");
      BtnExit->setGeometry(20,10,260,40);
      QWidget *qw=new QWidget(this);
      qw->setGeometry(40,50,200,90);
      QVBoxLayout *qb=new QVBoxLayout(qw);
      qb->addWidget(BtnExit);
      this->setCentralWidget(qw);
      qw->setLayout(qb);

}

MainWindow::~MainWindow()
{
    delete ui;
}
