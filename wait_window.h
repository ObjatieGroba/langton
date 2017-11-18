#ifndef WAIT_WINDOW_H
#define WAIT_WINDOW_H

#include <QWidget>

namespace Ui {
class Wait_window;
}

class Wait_window : public QWidget
{
    Q_OBJECT

public:
    explicit Wait_window(QWidget *parent = 0);
    ~Wait_window();

private:
    Ui::Wait_window *ui;
};

#endif // WAIT_WINDOW_H
