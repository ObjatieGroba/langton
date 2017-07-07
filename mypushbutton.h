#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>
#include <QString>

class MyPushButton : public QPushButton
{
public:
    MyPushButton(QWidget *parent);
    MyPushButton(const QString & text, QWidget * parent = 0);
protected:
    virtual void mousePressEvent(QMouseEvent* event);
};

#endif // MYPUSHBUTTON_H
