#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QVector> //顺序容器
#include <QPainter> //画图笔
#include <QTimer> //定时器
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void paintEvent(QPaintEvent *);
public slots:

private slots:
    void on_OpenSerialButton_clicked();
    void ReadData();
    void on_pushButton_clicked();
    void Timeout_redraw(void); //这是自己创建的定时器函数
    void Record_Wave(void);
    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *Timeout_redraw_timer;
    QTimer *Record_Wave_timer;


};

#endif // MAINWINDOW_H
