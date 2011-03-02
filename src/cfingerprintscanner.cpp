#include "cfingerprintscanner.h"

#include <QDebug>

CFingerprintScanner::CFingerprintScanner(QObject *parent) : QThread(parent)
{
    discv_devs = NULL; /* Found scanners list */
    using_dev = NULL; /* Device for using */
    dev = NULL;
    print_data = NULL;
    image = NULL; /* Fingerprint image */

    qimage = new QImage();

    connect(this, SIGNAL(errorSignal(QString)), this, SLOT(errorSlot(QString)));

    done = false;

    deviceOpened = false;
    scanInProgress = false;
    threadRunning = false;
    libInitialized = false;
    //exec();

    connect(this, SIGNAL(scanSuccessSignal()), this, SLOT(scanStop()));
    connect(this, SIGNAL(errorFatalSignal()), this, SLOT(scanStop()));
    connect(this, SIGNAL(finished()), this, SLOT(threadFinished()));
}

CFingerprintScanner::~CFingerprintScanner()
{
    delete qimage;
}

QByteArray CFingerprintScanner::createPGM(struct fp_img *img)
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

void CFingerprintScanner::threadStart()
{
    if(this->libInitialized)
    {
        if(!threadRunning)
        {
            done = false;
            threadRunning = true;
            this->start();
            this->getDevicesList();
        }
        else
        {
            emit errorSignal("This thread already running");
        }
    }
    else
    {
        emit errorSignal("Library not initialized");
    }
}

void CFingerprintScanner::threadStop()
{
    if(threadRunning)
    {
        qDebug() << "stopping thread";
        done = true;
        this->quit();
    }
    else
    {
        emit errorSignal("Thread not running yet");
    }
}

void CFingerprintScanner::libInit()
{
    /* Инициализируем библиотеку fprint */
    if (fp_init()) {
            emit errorSignal("Cannit init fprint library!\n");
    }
    else
    {
        this->libInitialized = true;
        emit this->libInitializedSignal();
    }
}

void CFingerprintScanner::libClose()
{
    fp_exit();

    this->libInitialized = false;
    emit this->libClosedSignal();
}

void CFingerprintScanner::getDevicesList()
{
    /* Getting list of system's devices */
    discv_devs = fp_discover_devs();
    if (!discv_devs) {
            emit errorSignal("Cannot discover any fingerprint device!\n");
            fp_exit();
    }
    else
    {
        emit this->deviceListReceivedSignal();
    }
}

void CFingerprintScanner::deviceOpen()
{
    if(!this->threadRunning)
    {
        this->threadStart();
    }

    // Using first founded device and print it's name
    //
    using_dev = discv_devs[0];
    qDebug() << "Found device using " << fp_driver_get_full_name(fp_dscv_dev_get_driver(using_dev)) << " driver.";
    if ((dev = fp_dev_open(using_dev)) == NULL) {
            emit errorSignal("Could not open device\n");
            fp_exit();
    }
    else
    {
        emit this->deviceOpenedSignal();
    }
}

void CFingerprintScanner::deviceClose()
{
    fp_dev_close(dev);

    emit this->deviceClosedSignal();
}

void CFingerprintScanner::run()
{
    this->mainLoop();
}

void CFingerprintScanner::getImage()
{
    if(image)
    {
        if(!qimage->loadFromData(this->createPGM(image), "PGM"))
        {
            emit errorSignal("error: image null!");
        }
        else
        {
            emit imageSignal(*qimage);
        }

        fp_img_free(image);
    }
}

void CFingerprintScanner::errorSlot(QString error)
{
    qDebug() << error;
}

void enroll_stage_cb(struct fp_dev *dev, int result,
        struct fp_print_data *print, struct fp_img *img, void *user_data)
{
    switch (result) {
            case FP_ENROLL_COMPLETE:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Enroll complete!\n");

                    if (!print) {
                            emit ((CFingerprintScanner*)user_data)->errorSignal("Enroll complete but no print?\n");
                            fp_dev_close(dev);
                            fp_exit();
                    }

                    ((CFingerprintScanner*)user_data)->image = img;

                    ((CFingerprintScanner*)user_data)->getImage();

                    emit ((CFingerprintScanner*)user_data)->scanSuccessSignal();

                    break;
            case FP_ENROLL_PASS:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Enroll stage passed.\n");
                    break;
            case FP_ENROLL_RETRY:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Didn't quite catch that. Please try again.\n");
                    break;
            case FP_ENROLL_RETRY_TOO_SHORT:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Your swipe was too short, please try again.\n");
                    break;
            case FP_ENROLL_RETRY_CENTER_FINGER:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Didn't catch that, please center your finger on the\
sensor and try again.\n");
                    break;
            case FP_ENROLL_RETRY_REMOVE_FINGER:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Scan failed, please remove your finger and then try\
again.\n");
                    break;
            case FP_ENROLL_FAIL:
                    emit ((CFingerprintScanner*)user_data)->errorSignal("Enroll failed, something wen't wrong :(\n");
                    emit ((CFingerprintScanner*)user_data)->errorFatalSignal();
    } // switch (enroll_retval)
}

void dev_open_cb(struct fp_dev *fpdev, int status, void *user_data)
{
        qDebug() << "device opened!" << status << endl;

        ((CFingerprintScanner*)user_data)->dev = fpdev;

        ((CFingerprintScanner*)user_data)->deviceOpened = true;

        emit ((CFingerprintScanner*)user_data)->deviceOpenedSignal();
}

void dev_close_cb(struct fp_dev *fpdev, void *user_data)
{
        qDebug() << "device closed!" << endl;

        //dev = fpdev;
        ((CFingerprintScanner*)user_data)->deviceOpened = false;
        ((CFingerprintScanner*)user_data)->scanInProgress = false;

        emit ((CFingerprintScanner*)user_data)->deviceClosedSignal();
}

void enroll_stopped(struct fp_dev *dev, void *user_data)
{
    qDebug() << "enroll stopped";
    ((CFingerprintScanner*)user_data)->scanInProgress = false;

    ((CFingerprintScanner*)user_data)->scanStoppedSignal();
}

void CFingerprintScanner::deviceOpenAPI()
{
    using_dev = discv_devs[0];

    qint8 r = fp_async_dev_open(this->using_dev, dev_open_cb, this);
    if(r < 0)
    {
        emit errorSignal("Could not open device");
        fp_exit();
    }
}

void CFingerprintScanner::deviceOpenAsync()
{
    if(!this->threadRunning)
    {
        this->threadStart();
    }

    if(!deviceOpened)
    {
        qDebug() << "opening device";

        this->deviceOpenAPI();

    }
    else
    {
        emit errorSignal("Device already opened");
    }
}

void CFingerprintScanner::deviceCloseAPI()
{
    fp_async_dev_close(this->dev, dev_close_cb, this);
}

void CFingerprintScanner::deviceCloseAsync()
{
    if(deviceOpened)
    {
        qDebug() << "closing device";

        this->deviceCloseAPI();
    }
    else
    {
        emit errorSignal("Device not opened");
    }
}

void CFingerprintScanner::scanStartAPI()
{
        qint8 r = fp_async_enroll_start(this->dev, enroll_stage_cb, this);
        if (r < 0)
        {
          emit errorSignal("Failed to start enrollment, error " + r);
        }
        else
        {
            this->scanInProgress = true;
            emit this->scanStartedSignal();
        }
}

void CFingerprintScanner::scanStopAPI()
{
        fp_async_enroll_stop(this->dev, enroll_stopped, this);
}

void CFingerprintScanner::scanStart()
{
    if(!this->threadRunning)
    {
        this->threadStart();
    }

    if(deviceOpened && !scanInProgress)
    {
        this->scanStartAPI();
    }
    else
    {
        if(!deviceOpened)
        {
            emit errorSignal("Device not opened");
        }
        else
        {
            emit errorSignal("Scan already started");
        }
    }
}

void CFingerprintScanner::scanStop()
{
    if(scanInProgress)
    {
        this->scanStopAPI();
    }
    else
    {
        emit errorSignal("Scan not started");
    }
}

void CFingerprintScanner::mainLoop()
{
    while(fp_handle_events() == 0 && !done);
}

void CFingerprintScanner::threadFinished()
{
    this->threadRunning = false;
    qDebug("Thread finished");
}
