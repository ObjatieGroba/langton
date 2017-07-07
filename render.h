#ifndef RENDER_H
#define RENDER_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <vector>

//QT_BEGIN_NAMESPACE
//class QImage;
//QT_END_NAMESPACE

class RenderThread : public QThread
{
    Q_OBJECT

public:
    RenderThread(QObject *parent = 0);
    ~RenderThread();

    void set_net(bool checked);
    void render(double centerX, double centerY, double scaleFactor, QSize resultSize, bool need_restart = true);
    void set_data(std::vector<std::vector<char>>* data, unsigned int * AntX, unsigned int * AntY, unsigned int * AntWay, bool * sync, size_t * did_steps, size_t * need_steps);

signals:
    void renderedImage(const QImage &image, double scaleFactor);

protected:
    void run() override;

private:
    QMutex mutex_r;
    QImage image;
    bool new_image;
    bool need_net;
    QWaitCondition condition;
    double centerX;
    double centerY;
    double scaleFactor;
    QSize resultSize;
    bool restart;
    bool abort;
    std::vector<unsigned int> colors;
    std::vector<std::vector<char>>* data;
    unsigned int* AntX;
    unsigned int* AntY;
    unsigned int* AntWay;
    size_t * did_steps;
    size_t * need_steps;
    int mem_iX;
    int mem_iY;
    unsigned int mem_AntX;
    unsigned int mem_AntY;
    bool* sync;
    QImage AntImage;
};

#endif // RENDER_H
