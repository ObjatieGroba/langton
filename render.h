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
    void set_trTile(bool checked);
    void set_colorTrTile(bool checked);
    void set_fillTrTile(bool checked);
    void set_arrowTrTile(bool checked);
    void stop();
    void clear_image();
    void render(double centerX, double centerY, double scaleFactor, QSize resultSize, bool need_restart = true, bool special = false);
    void set_data(std::vector<std::vector<char>>* data, std::vector<bool>* ways,
                  unsigned int * AntX, unsigned int * AntY, unsigned int * AntWay, unsigned int * steps,
                  bool * sync, size_t * did_steps, size_t * need_steps);

signals:
    void renderedImage(const QImage &image, double scaleFactor);
    void specialRender(const QImage &image);

protected:
    void run() override;

private:
    QMutex mutex_r;

    QImage image;

    bool new_image;
    bool need_net;
    bool arrowTrTile;
    bool trTile;
    bool colorTrTile;
    bool fillTrTile;
    bool otherEmit;
    bool restart;
    bool abort;
    bool need_stop;

    QWaitCondition condition;

    double centerX;
    double centerY;
    double scaleFactor;

    QSize resultSize;

    std::vector<unsigned int> colors;
    std::vector<std::vector<char>>* data;
    std::vector<bool>* ways;

    unsigned int* AntX;
    unsigned int* AntY;
    unsigned int* AntWay;
    unsigned int* steps;

    size_t * did_steps;
    size_t * need_steps;

    int mem_iX;
    int mem_iY;

    unsigned int mem_AntX;
    unsigned int mem_AntY;

    bool* sync;
    QImage AntImage;

    QImage AntImageT;
    QImage AntImageF;
};

#endif // RENDER_H
