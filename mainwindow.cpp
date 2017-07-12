#include <QPainter>
#include <QKeyEvent>
#include <math.h>
#include <string>
#include <vector>
#include "mainwindow.h"
#include "ui_mainwindow.h"

const unsigned int DefaultSize = 30000;
const double DefaultCenterX = DefaultSize / 2;
const double DefaultCenterY = DefaultSize / 2;
const double DefaultScale = 0.125f;
const unsigned int DefaultColorsNum = 2;
const unsigned int DefaultAntX = DefaultSize / 2, DefaultAntY = DefaultSize / 2;
const unsigned int DefaultAntWay = 0;

const double ZoomInFactor = 0.9f;
const double ZoomOutFactor = 1 / ZoomInFactor;
const int ScrollStep = 20;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    centerX = DefaultCenterX;
    centerY = DefaultCenterY;
    pixmapScale = DefaultScale;
    curScale = DefaultScale;
    AntWay = DefaultAntWay;
    AntX = DefaultAntX;
    AntY = DefaultAntY;
    did_steps = 0;
    need_steps = 0;
    sync = true;
    painting = false;
    pressing = false;
    ColorsNum = DefaultColorsNum;
    data = std::vector<std::vector<char>> (DefaultSize, std::vector<char> (DefaultSize, 0));
    ways = std::vector<bool> (ColorsNum, false);
    ways[0] = true;
    rules = new QLabel(this);
    statusBar()->addWidget(rules, 1);
    update_rules();
    paintB = new MyPushButton("Drag mode", this);
    connect(paintB, SIGNAL(clicked()),this, SLOT(paintSetClicked()));
    statusBar()->addWidget(paintB, 0);

    connect(&thread_r, SIGNAL(renderedImage(QImage,double)), this, SLOT(updatePixmap(QImage,double)));
    thread_r.set_data(&data, &ways, &AntX, &AntY, &AntWay, &sync, &did_steps, &need_steps);
    connect(&thread_a, SIGNAL(did()), this, SLOT(render_again()));
    thread_a.set_data(&data, &ways, &ColorsNum, &AntX, &AntY, &AntWay, &did_steps, &need_steps, &sync);

    setWindowTitle(tr("Langton's ant"));
#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif

    connect(&settings, SIGNAL(new_rule(std::vector<bool>)), this, SLOT(set_rule(std::vector<bool>)));
    std::string s;
    for (size_t i = 0; i != ColorsNum; ++i) {
        if (ways[i]) {
            s += "R";
        } else {
            s += "L";
        }
    }
    settings.set_rule(QString(s.c_str()));
    resize(1000, 500);
}

MainWindow::~MainWindow() {
    delete ui;
    thread_a.stop();
}

void MainWindow::start_action() {
    thread_a.go();
}

void MainWindow::pause(bool end) {
    thread_a.stop(end);
    if (ui->startstopButton->isChecked()) {
        ui->startstopButton->setChecked(false);
    }
}

void MainWindow::update_rules() {
    std::string s = "Rule: ";
    for (size_t i = 0; i != ColorsNum; ++i) {
        if (ways[i]) {
            s += "R";
        } else {
            s += "L";
        }
    }
    s += "; Num colors: " + std::to_string(ColorsNum);
    rules->setText(s.c_str());
}

void MainWindow::render_again() {
    thread_r.render(centerX, centerY, curScale, size(), need_steps == did_steps);
}

void MainWindow::set_rule(std::vector<bool> rule) {
    ColorsNum = static_cast<unsigned int>(rule.size());
    //on_SyncButton_clicked(true);
    //ui->SyncButton->setChecked(true);
    thread_a.clear();
    curScale = DefaultScale;
    centerY = centerX = static_cast<unsigned int>(data.size()) / 2;
    ways = rule;
    update_rules();
    render_again();
    set_active();
}

void MainWindow::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (pixmap.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
        return;
    }

    if (curScale == pixmapScale) {
        painter.drawPixmap(pixmapOffset, pixmap);
    } else {
        double scaleFactor = pixmapScale / curScale;
        int newWidth = int(pixmap.width() * scaleFactor);
        int newHeight = int(pixmap.height() * scaleFactor);
        int newX = pixmapOffset.x() + (pixmap.width() - newWidth) / 2;
        int newY = pixmapOffset.y() + (pixmap.height() - newHeight) / 2;

        painter.save();
        painter.translate(newX, newY);
        painter.scale(scaleFactor, scaleFactor);
        QRectF exposed = painter.matrix().inverted().mapRect(rect()).adjusted(-1, -1, 1, 1);
        painter.drawPixmap(exposed, pixmap, exposed);
        painter.restore();
    }

    QString text = tr(("Cur: " + std::to_string(did_steps) + "; Wanted: " + std::to_string(need_steps)).c_str());
    QFontMetrics metrics = painter.fontMetrics();
    int textWidth = metrics.width(text);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 127));
    painter.drawRect((width() - textWidth) / 2 - 5, 0, textWidth + 10, metrics.lineSpacing() + 5);
    painter.setPen(Qt::white);
    painter.drawText((width() - textWidth) / 2, metrics.leading() + metrics.ascent(), text);
}

void MainWindow::resizeEvent(QResizeEvent * /* event */)
{
    thread_r.render(centerX, centerY, curScale, size());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        scroll(-ScrollStep, 0);
        break;
    case Qt::Key_Right:
        scroll(+ScrollStep, 0);
        break;
    case Qt::Key_Down:
        scroll(0, +ScrollStep);
        break;
    case Qt::Key_Up:
        scroll(0, -ScrollStep);
        break;
    case Qt::Key_Plus:
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            zoom(ZoomInFactor);
            break;
        }
        pause(true);
        need_steps += 1;
        start_action();
        break;
    case Qt::Key_Minus:
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            zoom(ZoomOutFactor);
            break;
        }
        pause(true);
        if (need_steps > 0) {
            need_steps -= 1;
            start_action();
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

#ifndef QT_NO_WHEELEVENT
void MainWindow::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    double numSteps = numDegrees / 15.0f;
    zoom(pow(ZoomInFactor, numSteps));
}
#endif

void MainWindow::mousePressEvent(QMouseEvent *event) {
    pressing = true;
    if (!painting) {
        (*this).setCursor(Qt::ClosedHandCursor);
        if (event->button() == Qt::LeftButton)
            lastDragPos = event->pos();
    } else {
        lastClickPos = event->pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (!pressing) {
        return;
    }
    if (!painting) {
        if (event->buttons() & Qt::LeftButton) {
            pixmapOffset += event->pos() - lastDragPos;
            lastDragPos = event->pos();
            update();
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (!pressing) {
        return;
    }
    if (painting) {
        if (lastClickPos == event->pos()) {
            char num(1);
            bool minus = false;
            if (event->button() == Qt::LeftButton) {
                minus = false;
            } else if (event->button() == Qt::RightButton) {
                minus = true;
            } else {
                return;
            }
            int halfWidth = size().width() / 2;
            int halfHeight = size().height() / 2;
            thread_a.change_point(long((lastClickPos.x() - halfWidth) * curScale + centerX),
                                  long((lastClickPos.y() - halfHeight) * curScale + centerY), num, minus);
            render_again();
        }
    } else {
        (*this).setCursor(Qt::ArrowCursor);
        if (event->button() == Qt::LeftButton) {
            pixmapOffset += event->pos() - lastDragPos;
            lastDragPos = QPoint();

            int deltaX = (width() - pixmap.width()) / 2 - pixmapOffset.x();
            int deltaY = (height() - pixmap.height()) / 2 - pixmapOffset.y();
            scroll(deltaX, deltaY);
        }
    }
    pressing = false;
}

void MainWindow::updatePixmap(const QImage &image, double scaleFactor) {
    start_action();
    if (!lastDragPos.isNull()) {
        return;
    }
    pixmap = QPixmap::fromImage(image);
    pixmapOffset = QPoint();
    lastDragPos = QPoint();
    pixmapScale = scaleFactor;
    this->setFocus();
    update();
}

void MainWindow::zoom(double zoomFactor)
{
    curScale *= zoomFactor;
    if (curScale < 0.0078125f) {
        curScale = 0.0078125f;
    } else if (curScale > 64.0f) {
        curScale = 64.0f;
    }
    update();
    thread_r.render(centerX, centerY, curScale, size());
}

void MainWindow::set_active() {
    this->setEnabled(true);
}

void MainWindow::scroll(int deltaX, int deltaY)
{
    centerX += deltaX * curScale;
    centerY += deltaY * curScale;
    update();
    thread_r.render(centerX, centerY, curScale, size());
}

void MainWindow::paintSetClicked() {
    painting = !painting;
    if (painting) {
        pause(true);
        paintB->setText("Paint mode");
    } else {
        paintB->setText("Drag mode");
    }
    this->setFocus();
}

void MainWindow::on_MapIn_clicked() {
    zoom(ZoomInFactor);
    this->setFocus();
}

void MainWindow::on_MapOut_clicked() {
    zoom(ZoomOutFactor);
    this->setFocus();
}

void MainWindow::on_netButton_clicked(bool checked)
{
    thread_r.set_net(checked);
    this->setFocus();
}

void MainWindow::on_settingsButton_clicked()
{
    pause(true);
    this->setFocus();
    this->setEnabled(false);
    settings.show();
}

void MainWindow::on_SyncButton_clicked(bool checked)
{
    thread_a.stop();
    sync = checked;
    if (sync) {
        ui->SyncButton->setText("Sync");
    } else {
        ui->SyncButton->setText("ASync");
        start_action();
        render_again();
    }
    this->setFocus();
}

void MainWindow::on_startstopButton_clicked(bool checked)
{
    if (checked) {
        need_steps = static_cast<size_t>(-1);
        start_action();
    } else {
        pause(true);
    }
}

void MainWindow::on_pushButton_clicked()
{
    centerX = AntX;
    centerY = AntY;
    thread_r.render(centerX, centerY, curScale, size());
}

void MainWindow::on_TruchetButton_clicked(bool checked)
{
    thread_r.set_trTile(checked);
    ui->ColorTruchetButton->setDisabled(!checked);
}

void MainWindow::on_ColorTruchetButton_clicked(bool checked)
{
    thread_r.set_colortrTile(checked);
}
