#include "mypushbutton.h"
#include <QKeyEvent>

MyPushButton::MyPushButton(QWidget *parent): QPushButton(parent) { ; }

MyPushButton::MyPushButton(const QString &text, QWidget *parent): QPushButton(text, parent) { ; }

void MyPushButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);
    QPushButton::mouseReleaseEvent(event);
}
