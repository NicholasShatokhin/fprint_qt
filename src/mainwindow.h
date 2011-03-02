#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <Magick++.h>
//using namespace Magick;
#include <QImage>

#include "cfingerprintscanner.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QImage qimage;

    CFingerprintScanner * scanner;

private slots:
    void setImage(QImage image);
    void appClosed();

    void deviceOpened();
    void deviceClosed();
    void scanStarted();
    void scanStopped();

signals:
    void deleteThread();
};

#endif // MAINWINDOW_H
