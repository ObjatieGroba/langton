#include "photosaver.h"
#include "ui_photosaver.h"
#include <QPalette>
#include <memory>
#include <QFileDialog>
#include <QMessageBox>

PhotoSaver::PhotoSaver(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PhotoSaver)
{
    ui->setupUi(this);

    cellSize = 4;
    filename = "image";
    correctFilename = true;
}

PhotoSaver::~PhotoSaver()
{
    delete ui;
}

void PhotoSaver::set_photo(const QImage& image) {
    ui->RenderButton->setEnabled(true);
    if (image.isNull()) {
        return;
    }
    this->image = image;
    ui->ImageView->setPixmap(QPixmap::fromImage(this->image.scaled(ui->ImageView->width(), ui->ImageView->height(), Qt::KeepAspectRatio)));
    ui->ImageView->update();
    ui->ResultSizeLabel->setText((std::to_string(this->image.width()) + "px X " + std::to_string(this->image.height()) + "px (~" +
                                  std::to_string((this->image.width() * this->image.height()) >> 8) + "Kb ram)").c_str());
}

void PhotoSaver::set_used_size(unsigned int w, unsigned int h, double centerX, double centerY) {
    mapHeight = static_cast<unsigned int>(h);
    mapWidth = static_cast<unsigned int>(w);
    long long cellsNum = static_cast<long long>(h) * static_cast<long long>(w);
    long long max = 400;
    max <<= 18;
    int num = std::min(int(sqrt(static_cast<double>((max / cellsNum)))), 64);
    ui->horizontalSlider->setMaximum(static_cast<int>(num));
    CenterX = centerX;
    CenterY = centerY;
}

void PhotoSaver::on_PhotoSaver_finished(int result)
{
    if (!(ui->RenderButton->isEnabled()) || !correctFilename) {
        this->show();
        return;
    }
    if (result == 1) {
        QString fileName = QFileDialog::getSaveFileName(this,
                tr("Open data"), "",
                tr("png (*.png);;All Files (*)"));
        if (!fileName.isEmpty()) {
            try {
                image.save(fileName);
            } catch (...) {
                QMessageBox::information(this, tr("Save data error"), tr("Smth went wrong"));
            }
        } else {
            this->show();
            return;
        }
    }
    emit finished();
}

void PhotoSaver::on_horizontalSlider_valueChanged(int value)
{
    cellSize = static_cast<unsigned int>(value);
    ui->sizeLabel->setText(("Cell size: " + std::to_string(cellSize) + "px").c_str());
}

void PhotoSaver::on_RenderButton_clicked()
{
    ui->RenderButton->setDisabled(true);
    QSize size(mapWidth * cellSize, mapHeight * cellSize);
    double scale = 1.0f / cellSize;
    emit specialRender(CenterX, CenterY, scale, size);
}
