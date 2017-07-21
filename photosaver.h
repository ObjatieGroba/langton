#ifndef PHOTOSAVER_H
#define PHOTOSAVER_H

#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <string>

namespace Ui {
class PhotoSaver;
}

class PhotoSaver : public QDialog
{
    Q_OBJECT

public:
    explicit PhotoSaver(QWidget *parent = 0);
    ~PhotoSaver();

    void set_photo(const QImage& image);
    void set_used_size(unsigned int w, unsigned int h, double centerX, double centerY);

private slots:
    void on_PhotoSaver_finished(int result);
    void on_horizontalSlider_valueChanged(int value);
    void on_RenderButton_clicked();

signals:
    void finished();
    void specialRender(double centerX, double centerY, double scale, QSize size);

private:
    Ui::PhotoSaver *ui;
    unsigned int cellSize;
    unsigned int mapWidth;
    unsigned int mapHeight;
    double CenterX;
    double CenterY;
    bool correctFilename;
    QImage image;
    std::string filename;
};

#endif // PHOTOSAVER_H
