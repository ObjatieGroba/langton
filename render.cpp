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
    trTile = false;
    colorTrTile = false;
    fillTrTile = false;
    otherEmit = false;
    need_stop = false;
    colors = {255, 212, 169, 126, 83, 40, 0};
    mem_iX = INT_MIN;
    mem_iY = 0;
    mem_AntX = 0;
    mem_AntY = 0;
    AntImage = QImage(":/images/ant2.png");
    AntImageT = QImage(":/images/antT.png");
    AntImageF = QImage(":/images/antF.png");
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

void RenderThread::set_trTile(bool checked) {
    new_image = true;
    trTile = checked;
    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void RenderThread::set_colorTrTile(bool checked) {
    new_image = true;
    colorTrTile = checked;
    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void RenderThread::set_fillTrTile(bool checked) {
    new_image = true;
    fillTrTile = checked;
    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void RenderThread::set_data(std::vector<std::vector<char>>* data, std::vector<bool>* ways,
                            unsigned int* AntX, unsigned int* AntY, unsigned int* AntWay,
                            bool* sync, size_t * did_steps, size_t * need_steps) {
    if (!isRunning()) {
        mutex_r.lock();
        this->data = data;
        this->AntX = AntX;
        this->AntY = AntY;
        this->AntWay = AntWay;
        this->need_steps = need_steps;
        this->did_steps = did_steps;
        this->sync = sync;
        this->ways = ways;
        mutex_r.unlock();
    }
}

void RenderThread::clear_image() {
    image = QImage();
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

void RenderThread::render(double centerX, double centerY, double scaleFactor, QSize resultSize, bool need_restart, bool special)
{
    QMutexLocker locker(&mutex_r);

    this->centerX = centerX;
    this->centerY = centerY;
    this->scaleFactor = scaleFactor;
    this->resultSize = resultSize;

    if (special) {
        otherEmit = true;
    }

    if (!isRunning()) {
        new_image = true;
        start(LowPriority);
    } else {
        new_image = restart = need_restart;
        condition.wakeOne();
    }
}

void RenderThread::stop() {
    need_stop = true;
}

void RenderThread::run() {
    forever {
        //qDebug("render");
        mutex_r.lock();
        QSize resultSize = this->resultSize;
        double scaleFactor = this->scaleFactor;
        double centerX = this->centerX;
        double centerY = this->centerY;
        bool need_net = this->need_net;
        bool trTile = this->trTile;
        bool colorTrTile = this->colorTrTile;
        bool fillTrTile = this->fillTrTile;
        mutex_r.unlock();

        if (scaleFactor > 0.2f) {
            need_net = false;
            trTile = false;
        }
        if (mem_iX == INT_MIN || !(*sync)) {
            new_image = true;
        }

        int halfWidth = resultSize.width() / 2;
        int halfHeight = resultSize.height() / 2;
        double oneSize = 1 / scaleFactor;
        double halfCyrcleMin;
        if (fillTrTile) {
            halfCyrcleMin = 0;
        } else {
            halfCyrcleMin = (1 - 1.5f * scaleFactor) * (1 - 1.5f * scaleFactor) / 4;
        }
        double halfCyrcleMax = (1 + 1.5f * scaleFactor) * (1 + 1.5f * scaleFactor) / 4;
        unsigned int antSize = int(oneSize);
        if (antSize == 0) {
            antSize = 1;
        }
        // Map drawing
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

                if (ay >= 0 && ay < static_cast<int>(data->size())) {
                    for (int x = -halfWidth; x < halfWidth + resultSize.width() % 2; ++x) {
                        double adx = centerX + (x * scaleFactor);
                        int ax = int(adx);
                        if (ax >= 0 && ax < static_cast<int>(data->size())) {
                            if (need_net && ((ady - static_cast<double>(ay)) < scaleFactor ||
                                             (adx - static_cast<double>(ax)) < scaleFactor)) {
                                *scanLine = qRgb(0, 0, 0);
                            } else if (trTile) {
                                int r = 0, g = 0, b = 0;
                                if (colorTrTile) {
                                    unsigned int color = (*data)[ax][ay] * 11;
                                    if (color != 0) {
                                        r = colors[color % 7];
                                        g = colors[(color / 7) % 7];
                                        b = colors[(color / 49) % 7];
                                    }
                                }
                                bool is_tile = false;
                                if (ax % 2 == ay % 2) {
                                    if ((*ways)[(*data)[ax][ay]]) {
                                        // V R
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                        //qDebug(std::to_string(r).c_str());
                                        //qDebug((std::to_string(adx) + " " + std::to_string(ax)).c_str());
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    } else {
                                        // V L
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                    (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    }
                                } else {
                                    if ((*ways)[(*data)[ax][ay]]) {
                                        // H R
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                    (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    } else {
                                        // H L
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    }
                                }
                                if (is_tile) {
                                    *scanLine = qRgb(r, g, b);
                                } else {
                                    *scanLine = qRgb(255, 255, 255);
                                }
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
        // If same
        if (!new_image) {
            for (int y = mem_iY - 1; y < mem_iY + static_cast<int>(antSize) + 1; ++y) {
                double ady = centerY + ((y - halfHeight) * scaleFactor);
                int ay = int(ady);
                if (ay >= 0 && ay < static_cast<int>(data->size()) && y >= 0 && y < resultSize.height()) {
                    for (int x = mem_iX - 1; x < mem_iX + static_cast<int>(antSize) + 1; ++x) {
                        uint *scanLine = reinterpret_cast<uint *>(image.scanLine(y));
                        scanLine += x;
                        double adx = centerX + ((x - halfWidth) * scaleFactor);
                        int ax = int(adx);
                        if (ax >= 0 && ax < static_cast<int>(data->size()) && x >= 0 && x < resultSize.width()) {
                            if (need_net && ((ady - static_cast<double>(ay)) < scaleFactor ||
                                             (adx - static_cast<double>(ax)) < scaleFactor)) {
                                *scanLine = qRgb(0, 0, 0);
                            } else if (trTile) {
                                int r = 0, g = 0, b = 0;
                                if (colorTrTile) {
                                    unsigned int color = (*data)[ax][ay] * 11;
                                    if (color != 0) {
                                        r = colors[color % 7];
                                        g = colors[(color / 7) % 7];
                                        b = colors[(color / 49) % 7];
                                    }
                                }
                                bool is_tile = false;
                                if (ax % 2 == ay % 2) {
                                    if ((*ways)[(*data)[ax][ay]]) {
                                        // V R
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                        //qDebug(std::to_string(r).c_str());
                                        //qDebug((std::to_string(adx) + " " + std::to_string(ax)).c_str());
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    } else {
                                        // V L
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                    (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    }
                                } else {
                                    if ((*ways)[(*data)[ax][ay]]) {
                                        // H R
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                    (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    } else {
                                        // H L
                                        double lenght = (ady - static_cast<double>(ay)) * (ady - static_cast<double>(ay)) +
                                                   (adx - static_cast<double>(ax)) * (adx - static_cast<double>(ax));
                                        if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                            is_tile = true;
                                        } else {
                                            lenght = (1 - ady + static_cast<double>(ay)) * (1 - ady + static_cast<double>(ay)) +
                                                (1 - adx + static_cast<double>(ax)) * (1 - adx + static_cast<double>(ax));
                                            if (lenght >= halfCyrcleMin && lenght <= halfCyrcleMax) {
                                                is_tile = true;
                                            }
                                        }
                                    }
                                }
                                if (is_tile) {
                                    *scanLine = qRgb(r, g, b);
                                } else {
                                    *scanLine = qRgb(255, 255, 255);
                                }
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
        // Ant drawing
        double dX = (static_cast<double>(*AntX) - centerX) / scaleFactor;
        double dY = (static_cast<double>(*AntY) - centerY) / scaleFactor;
        int iX = int(dX) + halfWidth;
        int iY = int(dY) + halfHeight;
        QImage AntImageLocal;
        if (antSize <= 4) {
            AntImageLocal = AntImageF;
        } else if (antSize <= 32) {
            AntImageLocal = AntImageT;
        } else {
            AntImageLocal = AntImage;
        }
        if (iX < resultSize.width() && iY < resultSize.height() &&
            iX > -static_cast<int>(antSize) && iY > -static_cast<int>(antSize)) {
            mem_AntX = *AntX;
            mem_AntY = *AntY;
            mem_iX = iX;
            mem_iY = iY;
            QPainter painter(&image);
            QTransform transform;
            transform.rotate(90 * (*AntWay));
            QImage AntImageTmp(AntImageLocal.scaled(QSize(antSize, antSize)).transformed(transform));
            QPoint point(iX, iY);
            painter.drawImage(point, AntImageTmp);
        }

        if (abort)
            return;
        if (!restart) {
            if (otherEmit) {
                otherEmit = false;
                emit specialRender(image);
                need_stop = true;
            } else {
                emit renderedImage(image, scaleFactor);
            }
        }

        mutex_r.lock();
        if (!restart) {
            if (*sync) {
                condition.wait(&mutex_r);
            } else {
                if (need_stop) {
                    need_stop = false;
                    condition.wait(&mutex_r);
                } else {
                    msleep(100);
                }
            }
        }
        restart = false;
        mutex_r.unlock();
    }
}
