#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->TextTry->setVisible(false);
    ui->lineEditTry->setVisible(false);
    ui->TextSize->setVisible(false);
    ui->lineEditSize->setVisible(false);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_checkBox_clicked(bool checked)
{
    ui->TextTry->setVisible(checked);
    ui->lineEditTry->setVisible(checked);
    ui->TextSize->setVisible(checked);
    ui->lineEditSize->setVisible(checked);
}

bool Dialog::check_num(const QString& s) {
    if (s.size() == 0 || s.size() > 32) {
        return false;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            return false;
        }
    }
    return true;
}

void Dialog::on_Dialog_finished(int result)
{
    if (result == 0) {
        emit canceled();
    } else {
        if (check_num(ui->lineLength->text()) &&
            check_num(ui->lineEditSteps->text()) &&
            (!ui->checkBox->isChecked() ||
            (check_num(ui->lineEditSize->text()) &&
            check_num(ui->lineEditTry->text())))) {
            unsigned long long st = ui->lineLength->text().toULongLong();
            unsigned long long l = ui->lineEditSteps->text().toULongLong();
            unsigned long long t = 0, s = 0;
            if (ui->checkBox->isChecked()) {
                s = ui->lineEditSize->text().toULongLong();
                t = ui->lineEditTry->text().toULongLong();
            }
            emit applied(l, st, s, t);
        } else {
            this->show();
        }
    }
}

void Dialog::on_lineLength_textChanged(const QString &s)
{
    if (s.size() == 0 || s.size() > 32) {
        ui->lineLength->setStyleSheet("background: rgb(255, 200, 200)");
        return;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            ui->lineLength->setStyleSheet("background: rgb(255, 200, 200)");
            return;
        }
    }
    ui->lineLength->setStyleSheet("");
}

void Dialog::on_lineEditSteps_textChanged(const QString &s)
{
    if (s.size() == 0 || s.size() > 32) {
        ui->lineEditSteps->setStyleSheet("background: rgb(255, 200, 200)");
        return;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            ui->lineEditSteps->setStyleSheet("background: rgb(255, 200, 200)");
            return;
        }
    }
    ui->lineEditSteps->setStyleSheet("");
}

void Dialog::on_lineEditSize_textChanged(const QString &s)
{
    if (s.size() == 0 || s.size() > 32) {
        ui->lineEditSize->setStyleSheet("background: rgb(255, 200, 200)");
        return;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            ui->lineEditSize->setStyleSheet("background: rgb(255, 200, 200)");
            return;
        }
    }
    ui->lineEditSize->setStyleSheet("");
}

void Dialog::on_lineEditTry_textChanged(const QString &s)
{
    if (s.size() == 0 || s.size() > 32) {
        ui->lineEditTry->setStyleSheet("background: rgb(255, 200, 200)");
        return;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            ui->lineEditTry->setStyleSheet("background: rgb(255, 200, 200)");
            return;
        }
    }
    ui->lineEditTry->setStyleSheet("");
}
