#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGraphicsScene>
#include <QDebug>
#include <QFile>
#include <QBuffer>

QByteArray create_pgm(struct fp_img *img)
{
    int width = fp_img_get_width(img);
    int height = fp_img_get_height(img);
    int size = width * height;
    unsigned char * data = fp_img_get_data(img);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    QString str;
    str.sprintf("P5 %d %d 255\n", width, height);

    buffer.write(QByteArray(str.toAscii()));
    buffer.write(QByteArray::fromRawData(reinterpret_cast < const char* > (data), size));
    buffer.close();

    return byteArray;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    ui->setupUi(this);


    scanner = new CFingerprintScanner(this);

    scanner->libInit();

    connect(this->ui->btnDeviceOpen, SIGNAL(clicked()), scanner, SLOT(deviceOpenAsync()));
    connect(this->ui->btnDeviceClose, SIGNAL(clicked()), scanner, SLOT(deviceCloseAsync()));

    connect(scanner, SIGNAL(deviceOpenedSignal()), this, SLOT(deviceOpened()));
    connect(scanner, SIGNAL(deviceClosedSignal()), this, SLOT(deviceClosed()));
    connect(scanner, SIGNAL(scanStartedSignal()), this, SLOT(scanStarted()));
    connect(scanner, SIGNAL(scanStoppedSignal()), this, SLOT(scanStopped()));

    connect(scanner, SIGNAL(imageSignal(QImage)), this, SLOT(setImage(QImage)));

    QGraphicsScene * scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

}

MainWindow::~MainWindow()
{
    scanner->libClose();
    delete ui;
}

void MainWindow::setImage(QImage image)
{
    qimage = image;
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(qimage));
}

void MainWindow::appClosed()
{
    scanner->threadStop();
    qDebug() << "app closed";
}

void MainWindow::deviceOpened()
{
    scanner->scanStart();
}

void MainWindow::deviceClosed()
{
    scanner->threadStop();
}

void MainWindow::scanStarted()
{
    qDebug() << "please, swipe your finger";
}

void MainWindow::scanStopped()
{
    scanner->deviceCloseAsync();
}
