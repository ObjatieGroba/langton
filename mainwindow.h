#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "render.h"
#include "action.h"
#include "settings.h"
#include "photosaver.h"
#include "mypushbutton.h"
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event) override;
#endif
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updatePixmap(const QImage &image, double scaleFactor);
    void render_again();
    void zoom(double zoomFactor);
    void paintSetClicked();
    void set_active();
    void specialRenderFinished(const QImage& image);
    void specialRenderStart(double centerX, double centerY, double scale, QSize size);
    void set_rule(std::vector<bool> rule);

    void on_MapIn_clicked();
    void on_MapOut_clicked();
    void on_netButton_clicked(bool checked);
    void on_settingsButton_clicked();
    void on_SyncButton_clicked(bool checked);
    void on_startstopButton_clicked(bool checked);
    void on_pushButton_clicked();
    void on_TruchetButton_clicked(bool checked);
    void on_ColorTruchetButton_clicked(bool checked);
    void on_FillTruchetButton_clicked(bool checked);
    void on_SavePic_clicked();
    void on_stepsInput_textChanged(const QString &arg1);
    void on_stepsInput_editingFinished();
    void on_ArrowTruchetButton_clicked(bool checked);
    void on_LoadMap_clicked();
    void on_SaveMap_clicked();

private:
    Ui::MainWindow *ui;

    void scroll(int deltaX, int deltaY);
    void update_rules();
    void pause(bool end=false);
    void start_action();

    Settings settings;
    PhotoSaver photoSaver;

    RenderThread thread_r;
    ActionThread thread_a;

    QPixmap pixmap;
    QPoint pixmapOffset;
    QPoint lastDragPos;
    QPoint lastClickPos;
    QLabel* rules;
    MyPushButton* paintB;

    bool painting;
    bool sync;
    bool pressing;

    double centerX;
    double centerY;
    double pixmapScale;
    double curScale;

    unsigned int ColorsNum;
    unsigned int AntX;
    unsigned int AntY;
    unsigned int AntWay;
    unsigned int steps;

    size_t did_steps;
    size_t need_steps;

    std::vector<std::vector<char>> data;
    std::vector<bool> ways;
};

#endif // MAINWINDOW_H
