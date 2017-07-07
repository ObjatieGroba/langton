#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("imageformats");
    paths.append("platforms");
    QCoreApplication::setLibraryPaths(paths);
    QApplication app(argc, argv);
    MainWindow widget;
    widget.setCursor(Qt::ArrowCursor);
    widget.show();
    return app.exec();
}
