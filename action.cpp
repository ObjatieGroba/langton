#include "action.h"

#include <QtWidgets>
#include <cmath>
#include <vector>
#include <algorithm>

ActionThread::ActionThread(QObject *parent) : QThread(parent) {
    abort = false;
    need_stop = false;
}

ActionThread::~ActionThread() {
    mutex_a.lock();
    abort = true;
    condition.wakeOne();
    mutex_a.unlock();
    wait();
}

void ActionThread::clear() {
    mutex_a.lock();
    for (size_t i = 0; i != data->size(); ++i) {
        std::fill((*data)[i].begin(), (*data)[i].end(), 0);
    }
    *AntWay = 0;
    *AntX = *AntY = static_cast<unsigned int>(data->size()) / 2;
    *need_steps = *did_steps = 0;
    mutex_a.unlock();
    emit did();
}

void ActionThread::go() {
    //QMutexLocker locker(&mutex_a);
    need_stop = false;
    //if (need_stop) {
    //    qDebug("need stop");
    //} else {
    //    qDebug("not need stop");
    //}
    if (*need_steps == *did_steps) {
        return;
    }
    if (!isRunning()) {
        start(LowPriority);
    } else {
        condition.wakeOne();
    }
}

void ActionThread::stop(bool end) {
    need_stop = true;
    if (end) {
        mutex_a.lock();
        *need_steps = *did_steps;
        mutex_a.unlock();
    }
}

void ActionThread::set_data(std::vector<std::vector<char>>* data, std::vector<bool>* ways, unsigned int * ColorsNum, unsigned int * AntX, unsigned int * AntY, unsigned int * AntWay, size_t * did_steps, size_t * need_steps, bool * sync) {
    mutex_a.lock();
    this->data = data;
    this->ways = ways;
    this->ColorsNum = ColorsNum;
    this->AntX = AntX;
    this->AntY = AntY;
    this->AntWay = AntWay;
    this->need_steps = need_steps;
    this->did_steps = did_steps;
    this->sync = sync;
    mutex_a.unlock();
}

void ActionThread::run() {
    forever {
        if (abort)
            return;
        if (*AntX == 0 || *AntY == 0 || *AntX == static_cast<unsigned int>(data->size() - 1) || *AntY == static_cast<unsigned int>(data->size()) - 1) {
            need_stop = true;
        }
        //qDebug((std::to_string(*need_steps) + " " + std::to_string(*did_steps)).c_str());
        mutex_a.lock();
        if (need_stop) {
            need_stop = false;
            //qDebug("wait");
            condition.wait(&mutex_a);
            //qDebug("waited");
            mutex_a.unlock();
            continue;
        }
        //qDebug("ok");
        if (*need_steps > *did_steps) {
            //qDebug("didaaa");
            ++(*did_steps);
            if ((*ways)[(*data)[*AntX][*AntY]]) {
                *AntWay += 1;
                *AntWay %= 4;
            } else {
                if (*AntWay == 0) {
                    *AntWay = 3;
                } else {
                    *AntWay -= 1;
                }
            }
            ++(*data)[*AntX][*AntY];
            (*data)[*AntX][*AntY] %= *ColorsNum;
            if (*AntWay == 0) {
                *AntY -= 1;
            } else if (*AntWay == 1) {
                *AntX += 1;
            } else if (*AntWay == 2) {
                *AntY += 1;
            } else if (*AntWay == 3) {
                *AntX -= 1;
            }
        } else if (*need_steps < *did_steps) {
            //qDebug("dideeee");
            --(*did_steps);
            if (*AntWay == 0) {
                *AntY += 1;
            } else if (*AntWay == 1) {
                *AntX -= 1;
            } else if (*AntWay == 2) {
                *AntY -= 1;
            } else if (*AntWay == 3) {
                *AntX += 1;
            }
            if ((*data)[*AntX][*AntY] == 0) {
                (*data)[*AntX][*AntY] = static_cast<char>(*ColorsNum - 1);
            } else {
                --(*data)[*AntX][*AntY];
            }
            if (!(*ways)[(*data)[*AntX][*AntY]]) {
                *AntWay += 1;
                *AntWay %= 4;
            } else {
                if (*AntWay == 0) {
                    *AntWay = 3;
                } else {
                    *AntWay -= 1;
                }
            }
        }
        //qDebug("didd");
        if (*sync) {
            emit did();
            need_stop = true;
        } else {
            if (*need_steps == *did_steps) {
                need_stop = true;
            }
        }
        mutex_a.unlock();
    }
}

void ActionThread::change_point(long x, long y, char num, bool minus) {
    mutex_a.lock();
    if (x >= 0 && x < data->size() && y >= 0 && y < data->size()) {
        if (minus) {
            while ((*data)[x][y] < num) {
                (*data)[x][y] += *ColorsNum;
            }
            (*data)[x][y] = ((*data)[x][y] - num) % (*ColorsNum);
        } else {
            (*data)[x][y] = ((*data)[x][y] + num) % (*ColorsNum);
        }
    }
    mutex_a.unlock();
}
