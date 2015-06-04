#ifndef QPUSHBUTTONEXIT_H
#define QPUSHBUTTONEXIT_H
#include <QPushButton>
#include <QToolButton>
#include <QPoint>


class QPushButtonExit:public QPushButton
{
public:
    QPushButtonExit(){
        this->setFlat(true);
        this->setMouseTracking(true);
                     }
    void mouseMoveEvent ( QMouseEvent * event )
    {
         QToolTip::showText(QPoint(20,20),"mouseMoveEvent");
        this->setStyleSheet("background-color: rgb(255, 0, 0);");
    }

};



#endif // QPUSHBUTTONEXIT_H
