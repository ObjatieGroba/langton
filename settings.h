#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <random>

namespace Ui {
class settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    void set_rule(QString& s);

private slots:
    void on_settings_finished(int result);
    void on_lineEdit_textChanged(const QString &s);

    void on_RandomButton_clicked();

signals:
    void new_rule(std::vector<bool> rule);

private:
    Ui::settings *ui;
    bool normal;
    std::vector<bool> rule;
    std::random_device rd;
};

#endif // SETTINGS_H
