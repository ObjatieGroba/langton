#include "settings.h"
#include "ui_settings.h"
#include <QString>
#include <vector>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
    normal = true;
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_settings_finished(int result)
{
    if (result == 1) {
        if (normal) {
            emit(new_rule(rule));
        } else {
            this->show();
        }
    } else {
        emit(canceled());
    }
}

void Settings::on_lineEdit_textChanged(const QString &s)
{
    rule.clear();
    if (s.size() < 2) {
        normal = false;
        ui->errorlabel->setText("Rule minimum size is 2");
        return;
    }
    if (s.size() > 255) {
        normal = false;
        ui->errorlabel->setText("Rule maximum size is 255");
        return;
    }
    for (auto c : s) {
        if (c == 'L' || c == 'l') {
            rule.push_back(false);
        } else if (c == 'R' || c == 'r') {
            rule.push_back(true);
        } else {
            normal = false;
            ui->errorlabel->setText("Incorect!");
            return;
        }
    }
    ui->errorlabel->setText("");
    normal = true;
}

void Settings::set_rule(QString &s) {
    ui->lineEdit->setText(s);
}

void Settings::on_RandomButton_clicked()
{
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(2, 255), dist2(0, 1);
    size_t num = dist(gen);
    std::string s;
    for (size_t i = 0; i != num; ++i) {
        if (dist2(gen) == 0) {
            s += "L";
        } else {
            s += "R";
        }
    }
    ui->lineEdit->setText(QString(s.c_str()));
}
