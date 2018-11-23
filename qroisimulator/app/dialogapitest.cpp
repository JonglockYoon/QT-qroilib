#include "dialogapitest.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogapitest.h"
#include "imgprocbase.h"
#include "common.h"
#include "QZXing.h"
#include <zxing/NotFoundException.h>
#include <zxing/ReaderException.h>
#include "geomatch.h"
#include <opencv2/ximgproc.hpp>

using namespace cv;

DialogApiTest::DialogApiTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogApiTest)
{
    ui->setupUi(this);

    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Application%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    outImg = nullptr;
    method = 0;

    ui->comboBoxSource0->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource0->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource0->setCurrentIndex(0);
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource1->insertItem(i, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource1->setCurrentIndex(0);


    connect(ui->comboBoxSource0, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource0(int)));
    connect(ui->comboBoxSource1, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource1(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));
}

DialogApiTest::~DialogApiTest()
{
}

void DialogApiTest::closeEvent(QCloseEvent *event)
{
    if (outImg)
        cvReleaseImage(&outImg);
    outImg = nullptr;
}

void DialogApiTest::changeRealtime(bool checked)
{
    bRealtime = checked;

    if (source0 == 0)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    }
}

void DialogApiTest::activatedComboBoxSource0(int act)
{
    source0 = act;

    ui->comboBoxSource0->clear();
    ui->comboBoxSource0->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource0->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource0->setCurrentIndex(act);
}

void DialogApiTest::activatedComboBoxSource1(int act)
{
    source1 = act;

    ui->comboBoxSource1->clear();
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource1->insertItem(i, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource1->setCurrentIndex(act);
}
void DialogApiTest::on_pushButtonClose_clicked()
{
    if (outImg)
        cvReleaseImage(&outImg);
    outImg = nullptr;
    hide();
}

//
// bRealtime 처리..
//
void DialogApiTest::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogApiTest::on_pushButtonExec_clicked()
{
    IplImage* iplImg2 = nullptr;
    if (theMainWindow->vecOutWidget.size() > source1)
    {
        OutWidget* pWidget1 = theMainWindow->vecOutWidget[source1];
        ViewOutPage* pView1 = pWidget1->mViewOutPage;
        if (pView1) {
            iplImg2 = pView1->getIplcolor();
        }
    }

    IplImage* iplImg = nullptr;
    if (source0 == 0)
    {
        ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
        if (viewMain) {
            iplImg = viewMain->getIplcolor();

            if (!iplImg)
                return;

            Qroilib::DocumentView* v = viewMain->currentView();
            if (v != nullptr) {
                RoiObject *pData = nullptr;
                for (const Layer *layer : v->mRoi->objectGroups()) {
                    const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
                    for (const Qroilib::RoiObject *roiObject : objectGroup) {
                        pData = (RoiObject *)roiObject;
                        break;
                    }
                }
                if (pData == nullptr) {
                    ExecApplication(iplImg, iplImg2);
                    return;
                }

                IplImage* croppedImage;
                QRectF rect = pData->bounds();	// Area로 등록된 ROI.
                rect.normalized();

                if (rect.left() < 0) rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= iplImg->width) rect.setRight(iplImg->width);
                if (rect.bottom() >= iplImg->height) rect.setBottom(iplImg->height);
                pData->setBounds(rect);

                cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
                cvSetImageROI(iplImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), iplImg->depth, iplImg->nChannels);
                cvCopy(iplImg, croppedImage);
                cvResetImageROI(iplImg);

                ExecApplication(croppedImage, iplImg2);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecApplication(iplImg, iplImg2);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplcolor();

            if (!iplImg)
                return;
            ExecApplication(iplImg, iplImg2);
        }
    }
}

void DialogApiTest::on_radioButtoncopyMakeBorder_clicked()
{
    method = 0;
}
void DialogApiTest::on_radioButtoncalcBackProject_clicked()
{
    method = 1;
}

void DialogApiTest::ExecApplication(IplImage* iplImg, IplImage* iplImg2)
{
    switch(method)
    {
    case 0:
        copyMakeBorderTest(iplImg);
        break;
    case 1:
        calcBackProjectTest(iplImg, iplImg2);
        break;
    }
    if (outImg != nullptr)
        theMainWindow->outWidget(mName, outImg);
}

void DialogApiTest::copyMakeBorderTest(IplImage* iplImg)
{
    Mat image=cvarrToMat(iplImg);
    Mat result(image.rows+20, image.cols+20, image.type(), Scalar(0));

    copyMakeBorder(image, result, 10, 10, 10, 10, BORDER_CONSTANT);
    imshow("BORDER_CONSTANT", result);
    copyMakeBorder(image, result, 10, 10, 10, 10, BORDER_DEFAULT);
    imshow("BORDER_DEFAULT", result);
    copyMakeBorder(image, result, 10, 10, 10, 10, BORDER_ISOLATED);
    imshow("BORDER_ISOLATED", result);
    copyMakeBorder(image, result, 10, 10, 10, 10, BORDER_REFLECT);
    imshow("BORDER_REFLECT", result);
    copyMakeBorder(image, result, 10, 10, 10, 10, BORDER_REFLECT101);
    imshow("BORDER_REFLECT101", result);
}

// 히스토그램 역투영
// ref : https://blog.naver.com/ttootp/221244067721
// ref : https://docs.opencv.org/2.4/modules/imgproc/doc/histograms.html?highlight=calcbackproject
// ref : http://vision0814.tistory.com/70
// ref : http://hongkwan.blogspot.com/2013/01/opencv-4-4-example.html
void DialogApiTest::calcBackProjectTest(IplImage* iplImg, IplImage* iplImg2)
{
    Mat ref_hsv=cvarrToMat(iplImg);
    Mat tar_hsv =cvarrToMat(iplImg2);

    // Quantize the hue to 30 levels
    // and the saturation to 32 levels
    int hbins = 30, sbins = 32;
    int histSize[] = {hbins, sbins};
    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 256 };
    const float* ranges[] = { hranges, sranges };
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0, 1};

    int dims = 2;

    if (ui->BPCB2->isChecked()) // Hue Only
    {
        dims = 1;
    } else {
        dims = 2;
    }

    cv::Mat hist;
    cvtColor(ref_hsv, ref_hsv, COLOR_BGR2HSV);
    //GaussianBlur(ref_hsv, ref_hsv, Size(3, 3), 0);

    calcHist(&ref_hsv,
          1, // Number of source images.
          channels, // List of the dims channels used to compute the histogram.
          cv::Mat(), // do not use mask
          hist,  // Output histogram
          dims, // dims
          histSize, // Array of histogram sizes in each dimension. BINS 값.
          ranges,  // Array of the dims arrays of the histogram bin boundaries in each dimension. Range값.
          true, // the histogram is uniform
          false
      );

    cv::normalize(hist, hist, 1.0);

    cv::Mat result;
    cvtColor(tar_hsv, tar_hsv, COLOR_BGR2HSV);
    //GaussianBlur(tar_hsv, tar_hsv, Size(3, 3), 0);
    calcBackProject(&tar_hsv,
         1, // Number of source images.
         channels,// The list of channels used to compute the back projection.
         hist,    // Input histogram that can be dense or sparse.
         result,  // Destination back projection array that is a single-channel array of the same size and depth as images[0].
         ranges,  // Array of arrays of the histogram bin boundaries in each dimension.
         255.0    // Optional scale factor for the output back projection.
    );

    cv::threshold(result, result, 35, 255, cv::THRESH_BINARY);
    cv::namedWindow("Back Project Result");
    cv::imshow("Back Project Result", result);
}

