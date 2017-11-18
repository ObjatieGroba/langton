#include "wait_window.h"
#include "ui_wait_window.h"

Wait_window::Wait_window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Wait_window)
{
    this->setWindowTitle("Wait");
    ui->setupUi(this);
}

Wait_window::~Wait_window()
{
    delete ui;
}
