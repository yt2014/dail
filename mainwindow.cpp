#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QTextCodec>

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

    //this->setWindowFlags(Qt::FramelessWindowHint);
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

}

MainWindow::~MainWindow()
{
    delete ui;
}
