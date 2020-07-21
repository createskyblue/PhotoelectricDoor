#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QVector> //顺序容器
#include <QTextEdit>
#include <Windows.h>
#include <QDesktopServices> //用于打开超链接
#define ReceiveDataLeng 256
#define Graphics_Frame_X 20
#define Graphics_Frame_Y 10
int ReceiveData[2][ReceiveDataLeng]; //传感器类型；数据瀑布流
QVector<int> vectorA;
QVector<int> vectorB;
int TXTmp[7];
long RefreshCount=0;
int CommaCount=0;
int ThrowPack=0;
bool KeepInertia=true; //数据包丢失时图表依旧刷新，但是继承上一次的数据，有利于保持图表精度，该选项无需人为调整!!!
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{

  Sleep(1500);//给启动画面延迟后打开主窗口

  ui->setupUi(this);
  setWindowTitle(tr("强基创发 Stm32光电门实验 V1.0"));
  setWindowIcon(QIcon("./img/logo.png"));
  //限制窗体大小，防止看到不该看到的东西，如DEBUG留下的控制窗体
  setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
  setFixedSize(this->width(), this->height());
  on_pushButton_clicked();
  //初始化变量
  Timeout_redraw_timer=new QTimer(this);
  Record_Wave_timer=new QTimer(this);
  //设置timer超时事件与处理函数连接起来
  connect(Timeout_redraw_timer,SIGNAL(timeout()),this,SLOT(Timeout_redraw()));
  connect(Record_Wave_timer,SIGNAL(timeout()),this,SLOT(Record_Wave()));
}

MainWindow::~MainWindow()
{
  delete ui;
}

bool CheckDataIntegrity() {
    return TXTmp[3]==TXTmp[0]+TXTmp[1]+TXTmp[2]+TXTmp[4]+TXTmp[5]+TXTmp[6];
}

void ReceiveDataRoll() {
    if (vectorA.size()>=ReceiveDataLeng) {
        //因为A和B两个容器是同时进行操作的，长度都是一致的，判断其中一个足够了
        vectorA.remove(0,1);
        vectorB.remove(0,1);
    }
    //验证校验码确保数据完整性，不完整则丢包
    if (CheckDataIntegrity()) {
        vectorA<<TXTmp[1];
        vectorB<<TXTmp[2];
    }else if (KeepInertia&&vectorA.size()>0) {
        //检测到数据不完整，但由于“保持惯性”已开启，自动继承最近一次接收到的有效数据
        qDebug()<<"容器长度"<<vectorA.size();
        vectorA<<vectorA.at(vectorA.size()-2);
        vectorB<<vectorB.at(vectorA.size()-2);
    }
}

void MainWindow::Timeout_redraw(){

    update();
    //repaint();
}

void MainWindow::Record_Wave(){
    if (CommaCount>=3) ReceiveDataRoll();
}
void MainWindow::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    QColor green(0,255,87);
    QColor red(255,53,0);
    QColor yellow(255,210,160);
    QPen penG(green);
    QPen penR(red);
    QPen penY(yellow);
    QBrush brush(QColor(17, 43, 56, 255)); //决定图表背景框的颜色
    painter.setBrush(brush);
    painter.setRenderHints(QPainter::Antialiasing); //进行平滑抗锯齿
    painter.translate(Graphics_Frame_X, Graphics_Frame_Y); //移动坐标原点
    painter.drawRect(0, 0, ReceiveDataLeng*2, 256); //框
    //绘制刻度线
    painter.setPen(penY);
    for (int y=60;y<256;y+=10)
        for (int x=0;x<vectorA.size()*2;x+=10)
            painter.drawPoint(x, y);
    //绘制波形图
    for (int i=0;i<vectorA.size()-1;i++) {
        painter.setPen(penG);
        painter.drawLine(QPointF(i*2, vectorA.at(i)/16), QPointF((i+1)*2, vectorA.at(i+1)/16));
        painter.setPen(penR);
        painter.drawLine(QPointF(i*2, vectorB.at(i)/16), QPointF((i+1)*2, vectorB.at(i+1)/16));
    }
    //QApplication::processEvents();

}




void MainWindow::on_OpenSerialButton_clicked()
{
  if (ui->OpenSerialButton->text() == tr("打开串口"))
  {
    serial = new QSerialPort;
    //设置串口名
    serial->setPortName(ui->PortBox->currentText());
    //打开串口
    serial->open(QIODevice::ReadWrite);
    //设置波特率
    serial->setBaudRate(QSerialPort::Baud115200);//设置波特率为11520
    serial->setDataBits(QSerialPort::Data8);//设置数据位8
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
    //设置流控制
    serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

    //关闭设置菜单使能
    ui->PortBox->setEnabled(false);
    ui->BaudBox->setEnabled(false);
    ui->OpenSerialButton->setText(tr("关闭串口"));
    //连接信号槽
    QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::ReadData);
    //启动定时器
    Timeout_redraw_timer->start(66); //限制画面帧率 节省资源 FPS=15
    Record_Wave_timer->start(1);
  }
  else
  {
    //关闭串口
    serial->clear();
    serial->close();
    serial->deleteLater();

    //恢复设置使能
    ui->PortBox->setEnabled(true);
    ui->BaudBox->setEnabled(true);
    ui->OpenSerialButton->setText(tr("打开串口"));
    //关闭定时器
    Timeout_redraw_timer->stop();
    Record_Wave_timer->stop();
  }
}




//读取接收到的信息
void MainWindow::ReadData()
{
  QByteArray buf;
  buf = serial->readAll();
  QString str,tmp;
  CommaCount=0;
  if (!buf.isEmpty())
  {
    str = tr(buf);
    //开始读取以逗号分割的字符串
    for (int i=0;i<str.length();i++) {
        if (str[i]==",") {
            if (tmp.toInt()>=0&&CommaCount<7) {
                if (CommaCount==0&&tmp.toInt()<800) {
                    break;
                }else TXTmp[CommaCount]=tmp.toInt();
            }else break;
            tmp="";
            CommaCount++;
           /* if (CommaCount>=3) {
                if (ThrowPack>50) {  //防止采样速率过快，选择性进行丢包
                ReceiveDataRoll();
                ThrowPack=0;
                }
                ThrowPack++;
            }*/
        }else tmp+=str[i];
    }
    //ui->label_5->setText(tmp);
    //ui->textEdit->append(str);
    if (CheckDataIntegrity()) {
    ui->label_5->setText(QString::number(TXTmp[0], 10).toUpper());
    ui->label_7->setText(QString::number(TXTmp[1], 10).toUpper());
    ui->label_8->setText(QString::number(TXTmp[2], 10).toUpper());
    ui->label_12->setText(QString::number(TXTmp[3], 10).toUpper());
    ui->label_20->setText(QString::number(TXTmp[4], 10).toUpper());
    ui->label_21->setText(QString::number(TXTmp[5], 10).toUpper());
    ui->label_22->setText(QString::number(TXTmp[6], 10).toUpper());
    }
      //ui->textEdit->clear();
  }

  buf.clear();
}


void MainWindow::on_pushButton_clicked()
{
  //查找可用的串口
  ui->PortBox->clear();
  foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
  {
    QSerialPort serial;
    serial.setPort(info);
    if (serial.open(QIODevice::ReadWrite))
    {
      ui->PortBox->addItem(serial.portName());
      serial.close();
    }
  }
  //设置波特率下拉菜单默认显示第0项
  ui->BaudBox->setCurrentIndex(4);
}


//计算加速度
void MainWindow::on_pushButton_2_clicked()
{
    QString t1 = ui->label_20->text();
    QString t2 = ui->label_21->text();
    QString t3 = ui->label_22->text();
    QString s2 = ui->S2->text();

    //触发彩蛋 如果输入的挡光片长度为2020就会打开项目链接
    //if (s2.toInt()==2020) QDesktopServices::openUrl(QUrl(QLatin1String("https://github.com/createskyblue/PhotoelectricDoor")));
    if (s2.toInt()==2020) QDesktopServices::openUrl(QUrl(QLatin1String("https://gitee.com/createskyblue/PhotoelectricDoor")));
    double v1,v2;
    v1=s2.toDouble()*10.0/t1.toDouble();
    v2=s2.toDouble()*10.0/t2.toDouble();
    double ObjectAcceleration;
    ObjectAcceleration=double(v2-v1)*1000/(t3.toDouble());
    qDebug()<<ObjectAcceleration;
    ui->OjA->setText(QString::number(ObjectAcceleration, '.', 7));
}
