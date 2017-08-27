#ifndef ACTION_H
#define ACTION_H

#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include "analyzer.h"
#include <vector>

class ActionThread : public QThread
{
    Q_OBJECT

public:
    ActionThread(QObject *parent = 0);
    ~ActionThread();

    std::pair<std::pair<unsigned int, unsigned int>, std::pair<double, double>> get_used_size();
    void stop(bool end=false);
    void go();
    void clear();
    void new_rand(unsigned long long size);
    void change_point(long x, long y, char num, bool minus);
    void set_steps(unsigned int steps);
    void set_data(std::vector<std::vector<char>>* data, std::vector<bool>* ways, unsigned int * ColorsNum,
                  unsigned int * AntX, unsigned int * AntY, unsigned int * AntWay,
                  long long * did_steps, long long * need_steps, bool * sync, Analyzer * analyzer);

    bool save_data(QDataStream& stream);
    bool load_data(QDataStream& stream);

signals:
    void did();
    void new_rule(std::vector<bool> rules);
    void show_and_restart();

protected:
    void run() override;

private:
    QMutex mutex_a;
    QWaitCondition condition;

    bool abort;
    bool need_stop;
    bool * sync;

    std::vector<std::vector<char>>* data;
    std::vector<bool>* ways;

    unsigned int steps;

    unsigned int * ColorsNum;
    unsigned int * AntX;
    unsigned int * AntY;
    unsigned int * AntWay;

    long long * did_steps;
    long long * need_steps;

    Analyzer * analyzer;
};

#endif // ACTION_H
