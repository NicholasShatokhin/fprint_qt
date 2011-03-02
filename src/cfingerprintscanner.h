#ifndef CFINGERPRINTSCANNER_H
#define CFINGERPRINTSCANNER_H

#include <QImage>
#include <QBuffer>
#include <QThread>

#include <libfprint/fprint.h>


void enroll_stage_cb(struct fp_dev *dev, int result,
        struct fp_print_data *print, struct fp_img *img, void *user_data);
void dev_open_cb(struct fp_dev *fpdev, int status, void *user_data);
void dev_close_cb(struct fp_dev *fpdev, void *user_data);
void enroll_stopped(struct fp_dev *dev, void *user_data);

class CFingerprintScanner : public QThread
{
    Q_OBJECT

protected:
    //virtual void run();
    void run();

public:
    CFingerprintScanner(QObject *parent = 0);
    ~CFingerprintScanner();

public slots:
    void errorSlot(QString);

    void libInit();
    void libClose();
    void deviceOpenAsync();
    void deviceCloseAsync();
    void deviceOpen();
    void deviceClose();
    void scanStart();
    void scanStop();
    void threadStart();
    void threadStop();

private:
    QImage * qimage;
    struct fp_dscv_dev ** discv_devs; /* List of found scanners */
    struct fp_dscv_dev * using_dev; /* Using device */
    struct fp_dev * dev;
    struct fp_print_data * print_data;
    struct fp_img * image; /* Fingerprint image */

    bool done;

    bool deviceOpened;
    bool scanInProgress;
    bool threadRunning;
    bool libInitialized;


    QByteArray createPGM(struct fp_img *img);

    void getDevicesList();
    void getImage();


    void scanStartAPI();
    void scanStopAPI();
    void deviceOpenAPI();
    void deviceCloseAPI();

    void mainLoop();

    friend void enroll_stage_cb(struct fp_dev *dev, int result,
            struct fp_print_data *print, struct fp_img *img, void *user_data);
    friend void dev_open_cb(struct fp_dev *fpdev, int status, void *user_data);
    friend void dev_close_cb(struct fp_dev *fpdev, void *user_data);
    friend void enroll_stopped(struct fp_dev *dev, void *user_data);

private slots:
    void threadFinished();

signals:
    void errorSignal(const QString/*qint16 errorCode*/);

    void imageSignal(QImage image);
    void deviceOpenedSignal();
    void deviceClosedSignal();
    void scanStartedSignal();
    void scanStoppedSignal();
    void libInitializedSignal();
    void libClosedSignal();
    void deviceListReceivedSignal();

    void scanSuccessSignal();
    void errorFatalSignal();
};

#endif // CFINGERPRINTSCANNER_H
