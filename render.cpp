#include "render.h"

#include <QtWidgets>
#include <cmath>
#include <vector>
#include <limits.h>

RenderThread::RenderThread(QObject *parent) : QThread(parent)
{
    restart = false;
    abort = false;
    new_image = true;
    need_net = false;
    colors = {255, 212, 169, 126, 83, 40, 0};
    mem_iX = INT_MIN;
    mem_iY = 0;
    mem_AntX = 0;
    mem_AntY = 0;
    AntImage = QImage(":/images/ant.png");
    if (AntImage.isNull()) {
        qDebug() << "NULL ANT IMAGE!";
    }
}

RenderThread::~RenderThread()
{
    mutex_r.lock();
    abort = true;
    condition.wakeOne();
    mutex_r.unlock();
    wait();
}

void RenderThread::set_data(std::vector<std::vector<char>>* data, unsigned int* AntX, unsigned int* AntY, unsigned int* AntWay, bool* sync, size_t * did_steps, size_t * need_steps) {
    if (!isRunning()) {
        mutex_r.lock();
        this->data = data;
        this->AntX = AntX;
        this->AntY = AntY;
        this->AntWay = AntWay;
        this->need_steps = need_steps;
        this->did_steps = did_steps;
        this->sync = sync;
        mutex_r.unlock();
    }
}

void RenderThread::set_net(bool checked) {
    new_image = true;
    need_net = checked;
    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void RenderThread::render(double centerX, double centerY, double scaleFactor, QSize resultSize, bool need_restart)
{
    QMutexLocker locker(&mutex_r);

    this->centerX = centerX;
    this->centerY = centerY;
    this->scaleFactor = scaleFactor;
    this->resultSize = resultSize;

    if (!isRunning()) {
        new_image = true;
        start(LowPriority);
    } else {
        new_image = restart = need_restart;
        condition.wakeOne();
    }
}

void RenderThread::run() {
    forever {
        mutex_r.lock();
        QSize resultSize = this->resultSize;
        double scaleFactor = this->scaleFactor;
        double centerX = this->centerX;
        double centerY = this->centerY;
        bool need_net = this->need_net;
        mutex_r.unlock();

        if (scaleFactor > 0.125f) {
            need_net = false;
        }

        if (mem_iX == INT_MIN || !(*sync)) {
            new_image = true;
        }

        int halfWidth = resultSize.width() / 2;
        int halfHeight = resultSize.height() / 2;
        if (new_image) {
            mem_iX = INT_MIN;
            image = QImage(resultSize, QImage::Format_RGB32);

            for (int y = -halfHeight; y < halfHeight + resultSize.height() % 2; ++y) {
                if (restart)
                    break;
                if (abort)
                    return;

                uint *scanLine =
                        reinterpret_cast<uint *>(image.scanLine(y + halfHeight));
                double ady = centerY + (y * scaleFactor);
                int ay = int(ady);

                if (ay >= 0 && ay < data->size()) {
                    for (int x = -halfWidth; x < halfWidth + resultSize.width() % 2; ++x) {
                        double adx = centerX + (x * scaleFactor);
                        int ax = int(adx);
                        if (ax >= 0 && ax < data->size()) {
                            if (need_net && ((ady - static_cast<double>(ay)) < scaleFactor ||
                                             (adx - static_cast<double>(ax)) < scaleFactor)) {
                                *scanLine = qRgb(0, 0, 0);
                            } else {
                                unsigned int color = (*data)[ax][ay] * 11;
                                *scanLine = qRgb(colors[color % 7], colors[(color / 7) % 7], colors[(color / 49) % 7]);
                            }
                        }
                        ++scanLine;
                    }
                }
            }
        }

        double oneSize = 1 / scaleFactor;
        unsigned int antSize = int(oneSize);
        if (oneSize - static_cast<double>(antSize) > 0.1f) {
            antSize += 1;
        }
        if (antSize == 0) {
            antSize = 1;
        }
        if (!new_image) {
            for (int y = mem_iY; y < mem_iY + static_cast<int>(antSize) + 1; ++y) {
                double ady = centerY + ((y - halfHeight) * scaleFactor);
                int ay = int(ady);
                if (ay >= 0 && ay < data->size() && y >= 0 && y < resultSize.height()) {
                    for (int x = mem_iX; x < mem_iX + static_cast<int>(antSize) + 1; ++x) {
                        uint *scanLine = reinterpret_cast<uint *>(image.scanLine(y));
                        scanLine += x;
                        double adx = centerX + ((x - halfWidth) * scaleFactor);
                        int ax = int(adx);
                        if (ax >= 0 && ax < data->size() && x >= 0 && x < resultSize.width()) {
                            if (need_net && ((ady - static_cast<double>(ay)) < scaleFactor ||
                                             (adx - static_cast<double>(ax)) < scaleFactor)) {
                                *scanLine = qRgb(0, 0, 0);
                            } else {
                                unsigned int color = (*data)[ax][ay] * 11;
                                *scanLine = qRgb(colors[color % 7], colors[(color / 7) % 7], colors[(color / 49) % 7]);
                            }
                        }
                        ++scanLine;
                    }
                }
            }
        }
        double dX = (static_cast<double>(*AntX) - centerX) / scaleFactor;
        double dY = (static_cast<double>(*AntY) - centerY) / scaleFactor;
        int iX = int(dX) + halfWidth;
        int iY = int(dY) + halfHeight;
        if (iX < resultSize.width() && iY < resultSize.height() && iX > -static_cast<int>(antSize) && iY > -static_cast<int>(antSize)) {
            mem_AntX = *AntX;
            mem_AntY = *AntY;
            mem_iX = iX;
            mem_iY = iY;
            QPainter painter(&image);
            QTransform transform;
            transform.rotate(90 * (*AntWay));
            QImage AntImageTmp(AntImage.scaled(QSize(antSize, antSize)).transformed(transform));
            QPoint point(iX, iY);
            painter.drawImage(point, AntImageTmp);
        }

        if (abort)
            return;
        if (!restart)
            emit renderedImage(image, scaleFactor);

        mutex_r.lock();
        if (!restart) {
            if (*sync) {
                condition.wait(&mutex_r);
            } else {
                msleep(100);
            }
        }
        restart = false;
        mutex_r.unlock();
    }
}
