#include <QPainter>
#include <QThread>
#include <QKeyEvent>
#include <math.h>
#include <string>
#include <vector>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <limits>

class Sleeper : public QThread
{
public:
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
};

const unsigned int DefaultSize = 10000;
const double DefaultCenterX = DefaultSize / 2;
const double DefaultCenterY = DefaultSize / 2;
const double DefaultScale = 0.125f;
const unsigned int DefaultColorsNum = 2;
const unsigned int DefaultAntX = DefaultSize / 2, DefaultAntY = DefaultSize / 2;
const unsigned int DefaultAntWay = 0;
const double ZoomInFactor = 0.9f;
const double ZoomOutFactor = 1 / ZoomInFactor;
const int ScrollStep = 20;
const unsigned long long DefaultCheckSum = 1000000;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    centerX = DefaultCenterX;
    AutoAnalyzerStepsNum = DefaultCheckSum;
    centerY = DefaultCenterY;
    pixmapScale = DefaultScale;
    curScale = DefaultScale;
    AntWay = DefaultAntWay;
    AntX = DefaultAntX;
    AntY = DefaultAntY;
    did_steps = 0;
    need_steps = 0;
    steps = 1;
    sync = true;
    painting = false;
    pressing = false;
    special = false;
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
    thread_r.set_data(&data, &ways, &AntX, &AntY, &AntWay, &steps, &sync, &did_steps, &need_steps);
    connect(&thread_a, SIGNAL(did()), this, SLOT(render_again()));
    thread_a.set_data(&data, &ways, &ColorsNum, &AntX, &AntY, &AntWay, &did_steps, &need_steps, &sync, &analyzer);

    connect(&thread_a, SIGNAL(show_and_restart()), this, SLOT(show_and_restart()));

    ui->ColorTruchetButton->setVisible(false);
    ui->FillTruchetButton->setVisible(false);
    ui->ArrowTruchetButton->setVisible(false);
    ui->Analyze->setVisible(false);
    ui->Auto->setVisible(false);

    setWindowTitle(tr("Langton's ant"));
#ifndef QT_NO_CURSOR
    setCursor(Qt::CrossCursor);
#endif

    connect(&settings, SIGNAL(new_rule(std::vector<bool>)), this, SLOT(set_rule(std::vector<bool>)));
    connect(&settings, SIGNAL(canceled()), this, SLOT(set_active()));

    connect(&photoSaver, SIGNAL(finished()), this, SLOT(set_active()));
    connect(&photoSaver, SIGNAL(specialRender(double,double,double,QSize)),
            this, SLOT(specialRenderStart(double,double,double,QSize)));

    connect(&thread_r, SIGNAL(specialRender(QImage)), this, SLOT(specialRenderFinished(QImage)));

    connect(&dialog, SIGNAL(canceled()), this, SLOT(AutoAnalyzerCanceled()));
    connect(&dialog, SIGNAL(applied(unsigned long long,unsigned long long,unsigned long long,unsigned long long)),
            this, SLOT(AutoAnalyzerStart(unsigned long long,unsigned long long,unsigned long long,unsigned long long)));

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
    thread_r.stop();
    thread_a.stop(end);
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
    analyzer.clear();
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
    uint32_t y = height() - ui->Stop->height() - 5 - ui->statusbar->height(), x = (width() - ui->Stop->width()) / 2;
    ui->Stop->setGeometry(x, y, 25, 25);
    ui->GoBack->setGeometry(x - 30, y, 25, 25);
    ui->GoForward->setGeometry(x + 30, y, 25, 25);

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
        if (analyzer.isAutoAnalyzerEnabled()) {
            return;
        }
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            zoom(ZoomInFactor);
            break;
        }
        pause(true);
        need_steps += steps;
        start_action();
        render_again();
        break;
    case Qt::Key_Minus:
        if (analyzer.isAutoAnalyzerEnabled()) {
            return;
        }
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            zoom(ZoomOutFactor);
            break;
        }
        pause(true);
        need_steps -= steps;
        start_action();
        render_again();
        break;
    case Qt::Key_Space:
        if (analyzer.isAutoAnalyzerEnabled()) {
            return;
        }
        pause(true);
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

void MainWindow::new_next_rule() {
    AutoAnalyzerTried = 0;
    std::vector<bool> copy = ways;
    size_t id = -1;
    for (size_t i = 0; i != ways.size(); ++i) {
        if (!ways[i]) {
            id = i;
        }
    }
    if (id == static_cast<size_t>(-1)) {
        copy.resize(ways.size() + 1);
        for (size_t i = 1; i != copy.size(); ++i) {
            copy[i] = false;
        }
        copy[0] = true;
    } else {
        copy[id] = true;
        while (++id < copy.size()) {
            copy[id] = false;
        }
    }
    set_rule(copy);
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
    if (special) {
        pause(true);
        ++AutoAnalyzerTried;
        special = false;
        ui->AutoAnalyzerText->setText(("Try: " + std::to_string(AutoAnalyzerTried) + "/" + std::to_string(AutoAnalyzerNumOfTry)).c_str());
        size_t id = analyzer.analyze();
        if (id != 0) {
            std::string rulestr = "";
            for (size_t i = 0; i != ColorsNum; ++i) {
                if (ways[i]) {
                    rulestr += "R";
                } else {
                    rulestr += "L";
                }
            }
            std::string msg = "Rule: " + rulestr + "\r\nLen: " + std::to_string(id) + "\r\nStat: " + analyzer.statistic(id, ColorsNum);

            QString filename = QString::fromStdString(rulestr) + ".jpg";
            image.save(filename);

            filename = QString::fromStdString(rulestr) + ".df";
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                    file.errorString());
                return;
            }
            QDataStream out(&file);
            thread_a.save_data(out);
            file.close();

            filename = QString::fromStdString(rulestr) + ".txt";
            QFile file2(filename);
            if (file2.open(QIODevice::ReadWrite)) {
                QTextStream stream(&file2);
                stream << QString::fromStdString(msg);
            }
            file2.close();
            new_next_rule();
        } else if (AutoAnalyzerNumOfTry != 0 && AutoAnalyzerTried < AutoAnalyzerNumOfTry) {
            thread_a.new_rand(AutoAnalyzerRandSize);
            analyzer.clear();
        } else {
            new_next_rule();
        }
        Sleeper::msleep(100);
        need_steps = AutoAnalyzerStepsNum;
        analyzer.setEnabled(true);
        analyzer.setAutoAnalyzer(true);
        start_action();
        render_again();
    }
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

void MainWindow::specialRenderFinished(const QImage &image) {
    photoSaver.set_photo(image);
}

void MainWindow::specialRenderStart(double centerX, double centerY, double scale, QSize size) {
    thread_r.render(centerX, centerY, scale, size, true, true);
}

void MainWindow::set_active() {
    this->setEnabled(true);
    render_again();
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

void MainWindow::on_pushButton_clicked()
{
    centerX = AntX + 0.5f;
    centerY = AntY + 0.5f;
    thread_r.render(centerX, centerY, curScale, size());
}

void MainWindow::on_TruchetButton_clicked(bool checked)
{
    thread_r.set_trTile(checked);
    ui->ColorTruchetButton->setVisible(checked);
    ui->FillTruchetButton->setVisible(checked);
    ui->ArrowTruchetButton->setVisible(checked);
}

void MainWindow::on_ColorTruchetButton_clicked(bool checked)
{
    thread_r.set_colorTrTile(checked);
}

void MainWindow::on_FillTruchetButton_clicked(bool checked)
{
    thread_r.set_fillTrTile(checked);
    this->setFocus();
}

void MainWindow::on_ArrowTruchetButton_clicked(bool checked)
{
    thread_r.set_arrowTrTile(checked);
    this->setFocus();
}

void MainWindow::on_SavePic_clicked()
{
    pause(true);
    this->setEnabled(false);
    photoSaver.set_photo(pixmap.toImage());
    std::pair<std::pair<unsigned int, unsigned int>, std::pair<double, double>> p = thread_a.get_used_size();
    photoSaver.set_used_size(p.first.first, p.first.second, p.second.first, p.second.second);
    photoSaver.show();
}

void MainWindow::on_stepsInput_textChanged(const QString &s)
{
    if (s.size() == 0) {
        ui->stepsInput->setStyleSheet("background: rgb(255, 200, 200)");
        return;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            ui->stepsInput->setStyleSheet("background: rgb(255, 200, 200)");
            return;
        }
    }
    ui->stepsInput->setStyleSheet("");
}

void MainWindow::on_stepsInput_editingFinished()
{
    QString s = ui->stepsInput->text();
    if (s.size() == 0) {
        return;
    }
    for (auto c : s) {
        if (c > '9' || c < '0') {
            return;
        }
    }
    steps = ui->stepsInput->text().toInt();
    thread_a.set_steps(steps);
}

void MainWindow::on_LoadMap_clicked()
{
    pause(true);
    this->setEnabled(false);
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open data"), "",
            tr("DataFile (*.df);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        QDataStream in(&file);
        if (!thread_a.load_data(in)) {
            QMessageBox::information(this, tr("Load data error"), tr("Data is not valid"));
        }
        ColorsNum = static_cast<unsigned int>(ways.size());
        update_rules();
        render_again();
    }
    this->setEnabled(true);
    render_again();
}

void MainWindow::on_SaveMap_clicked()
{
    pause(true);
    this->setEnabled(false);
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Open data"), "",
            tr("DataFile (*.df);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }
        QDataStream out(&file);
        if (!thread_a.save_data(out)) {
            QMessageBox::information(this, tr("Save data error"), tr("Smth went wrong"));
        }
    }
    this->setEnabled(true);
    render_again();
}

void MainWindow::on_OnOffAnalyzer_clicked(bool checked)
{
    pause(true);
    analyzer_enabled = checked;
    analyzer.setEnabled(checked);
    ui->Analyze->setVisible(checked);
    ui->Auto->setVisible(checked);
    if (checked) {
        ;
    } else {
        analyzer.clear();
    }
}

void MainWindow::on_Analyze_clicked()
{
    pause(true);
    size_t id = analyzer.analyze();
    QMessageBox::information(this, tr("Info"), ("Cur info: " + std::to_string(id) + "\n" + analyzer.statistic(id, ColorsNum)).c_str());
}

void MainWindow::on_Auto_clicked(bool checked)
{
    pause(true);
    if (checked) {
        dialog.show();
        this->setEnabled(false);
    } else {
        ui->AutoAnalyzerText->setText("");
        AutoAnalyzerButtonBlocker(true);
        analyzer.setAutoAnalyzer(false);
    }
}

void MainWindow::show_and_restart() {
    pause(true);
    if (!analyzer.isEnabled()) {
        return;
    }
    analyzer.setEnabled(false);
    analyzer.setAutoAnalyzer(false);
    on_pushButton_clicked();
    special = true;
}

void MainWindow::on_Stop_clicked()
{
    pause(true);
}

void MainWindow::on_GoBack_clicked()
{
    if (need_steps > 0) {
        need_steps = 0;
    } else {
        need_steps = LLONG_MIN;
    }
    render_again();
    start_action();
}

void MainWindow::on_GoForward_clicked()
{
    if (need_steps >= 0) {
        need_steps = LLONG_MAX;
    } else {
        need_steps = 0;
    }
    render_again();
    start_action();
}

void MainWindow::AutoAnalyzerCanceled() {
    ui->Auto->setChecked(false);
    set_active();
}

void MainWindow::AutoAnalyzerButtonBlocker(bool b) {
    ui->GoBack->setEnabled(b);
    ui->GoForward->setEnabled(b);
    ui->Stop->setEnabled(b);
    ui->Analyze->setEnabled(b);
    ui->OnOffAnalyzer->setEnabled(b);
    ui->SaveMap->setEnabled(b);
    ui->LoadMap->setEnabled(b);
    ui->SavePic->setEnabled(b);
    ui->settingsButton->setEnabled(b);
    ui->stepsInput->setEnabled(b);
}

void MainWindow::AutoAnalyzerStart(unsigned long long l, unsigned long long st, unsigned long long s, unsigned long long t) {
    AutoAnalyzerStepsNum = l;
    AutoAnalyzerRandSize = s;
    AutoAnalyzerNumOfTry = t;
    AutoAnalyzerTried = 0;
    analyzer.clear();
    analyzer.setDataLength(st * 2);
    need_steps = AutoAnalyzerStepsNum;
    analyzer.setAutoAnalyzer(true);
    set_active();
    AutoAnalyzerButtonBlocker(false);
    start_action();
    render_again();
}
