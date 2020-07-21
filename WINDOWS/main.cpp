#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap("./img/screen.png");
    QSplashScreen splash(pixmap);
    qDebug()<<pixmap;
    splash.show();
    splash.showMessage("程序:2020届 LHW-创天蓝", Qt::AlignBottom, QColor(238, 72, 42, 255));
    a.processEvents();

    MainWindow w;
    w.show();

    splash.finish(&w);
    return a.exec();
}
