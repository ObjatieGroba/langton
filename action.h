#ifndef ACTION_H
#define ACTION_H

#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
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
    void change_point(long x, long y, char num, bool minus);
    void set_steps(unsigned int steps);
    void set_data(std::vector<std::vector<char>>* data, std::vector<bool>* ways, unsigned int * ColorsNum,
                  unsigned int * AntX, unsigned int * AntY, unsigned int * AntWay,
                  size_t * did_steps, size_t * need_steps, bool * sync);

    bool save_data(QDataStream& stream);
    bool load_data(QDataStream& stream);

signals:
    void did();
    void new_rule(std::vector<bool> rules);

protected:
    void run() override;

private:
    QMutex mutex_a;
    QWaitCondition condition;

    bool abort;
    bool * sync;
    bool need_stop;

    std::vector<std::vector<char>>* data;
    std::vector<bool>* ways;

    unsigned int steps;

    unsigned int * ColorsNum;
    unsigned int * AntX;
    unsigned int * AntY;
    unsigned int * AntWay;

    size_t * did_steps;
    size_t * need_steps;
};

#endif // ACTION_H
