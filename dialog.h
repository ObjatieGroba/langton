#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_checkBox_clicked(bool checked);
    void on_Dialog_finished(int result);
    void on_lineLength_textChanged(const QString &arg1);
    void on_lineEditSteps_textChanged(const QString &arg1);
    void on_lineEditSize_textChanged(const QString &arg1);
    void on_lineEditTry_textChanged(const QString &arg1);

signals:
    void canceled();
    void applied(unsigned long long l, unsigned long long st, unsigned long long s, unsigned long long t);

private:
    Ui::Dialog *ui;
    bool check_num(const QString& s);
};

#endif // DIALOG_H
