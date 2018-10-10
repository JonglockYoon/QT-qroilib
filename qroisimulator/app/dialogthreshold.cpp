#include "dialogthreshold.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogthreshold.h"
#include "imgprocbase.h"
#include "common.h"

using namespace cv;
using namespace cv::xfeatures2d;

DialogThreshold::DialogThreshold(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogThreshold)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Threshold%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    method = 0;

    ui->setupUi(this);

    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(0);

    ui->comboBoxThresholdMethod->insertItem(0, QIcon(), QString::fromLocal8Bit("Adaptive"));
    ui->comboBoxThresholdMethod->insertItem(1, QIcon(), QString::fromLocal8Bit("Threshold"));
    ui->comboBoxThresholdMethod->insertItem(2, QIcon(), QString::fromLocal8Bit("OTSU"));
    ui->comboBoxThresholdMethod->insertItem(3, QIcon(), QString::fromLocal8Bit("HistogramEqualize"));
    ui->comboBoxThresholdMethod->insertItem(4, QIcon(), QString::fromLocal8Bit("AverageBright Inspection"));
    ui->comboBoxThresholdMethod->setCurrentIndex(method);

    // Adaptive group
    ui->slidera1->setRange(0, 300);
    ui->slidera1->setValue(WinSize);
    ui->slidera1->setSingleStep(1);
    ui->slidera2->setRange(0, 50);
    ui->slidera2->setValue(AreaSize);
    ui->slidera2->setSingleStep(1);

    // Threshold group
    ui->sliderth1->setRange(0, 255);
    ui->sliderth1->setValue(WinSize);
    ui->sliderth1->setSingleStep(1);
    ui->sliderth2->setRange(0, 255);
    ui->sliderth2->setValue(AreaSize);
    ui->sliderth2->setSingleStep(1);

    connect(ui->comboBoxSource, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource(int)));
    connect(ui->comboBoxThresholdMethod, SIGNAL(activated(int)), this, SLOT(activatedComboBoxThresholdMethod(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));
    QObject::connect(ui->chkBoxNot, SIGNAL(clicked(bool)), this, SLOT(changeNot(bool)));

    // Adaptive group
    QObject::connect(ui->slidera1, SIGNAL(valueChanged(int)), this, SLOT(setValuea1(int)));
    QObject::connect(ui->slidera2, SIGNAL(valueChanged(int)), this, SLOT(setValuea2(int)));
    QObject::connect(ui->edita1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuea1(const QString &)));
    QObject::connect(ui->edita2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuea2(const QString &)));
    str = QString("%1").arg(WinSize);
    ui->edita1->setText(str);
    str = QString("%1").arg(AreaSize);
    ui->edita2->setText(str);

    // threshold group
    QObject::connect(ui->sliderth1, SIGNAL(valueChanged(int)), this, SLOT(setValueth1(int)));
    QObject::connect(ui->sliderth2, SIGNAL(valueChanged(int)), this, SLOT(setValueth2(int)));
    QObject::connect(ui->editth1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValueth1(const QString &)));
    QObject::connect(ui->editth2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValueth2(const QString &)));
    str = QString("%1").arg(low);
    ui->editth1->setText(str);
    str = QString("%1").arg(high);
    ui->editth2->setText(str);

    ui->editAvgBr1->setText("10");
    ui->editAvgBr2->setText("100");
}

DialogThreshold::~DialogThreshold()
{
}

void DialogThreshold::closeEvent(QCloseEvent *event)
{
    if (source == 0)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    }

    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
}

void DialogThreshold::setValuea1(int val)
{
    WinSize = val;
    QString str = QString("%1").arg(val);
    ui->edita1->setText(str);
}
void DialogThreshold::setValuea2(int val)
{
    AreaSize = val;
    QString str = QString("%1").arg(val);
    ui->edita2->setText(str);
}

void DialogThreshold::setEditValuea1(const QString &val)
{
    int pos = val.toInt();
    ui->slidera1->setValue(pos);
}
void DialogThreshold::setEditValuea2(const QString &val)
{
    int pos = val.toInt();
    ui->slidera2->setValue(pos);
}

void DialogThreshold::setValueth1(int val)
{
    low = val;
    QString str = QString("%1").arg(val);
    ui->editth1->setText(str);
}
void DialogThreshold::setValueth2(int val)
{
    high = val;
    QString str = QString("%1").arg(val);
    ui->editth2->setText(str);
}
void DialogThreshold::setEditValueth1(const QString &val)
{
    low = val.toInt();
    ui->sliderth1->setValue(low);
}
void DialogThreshold::setEditValueth2(const QString &val)
{
    high = val.toInt();
    ui->sliderth2->setValue(high);
}

void DialogThreshold::changeNot(bool checked)
{
    bNot = checked;
}

void DialogThreshold::changeRealtime(bool checked)
{
    bRealtime = checked;

    if (source == 0)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    }
}

void DialogThreshold::activatedComboBoxSource(int act)
{
    source = act;

    ui->comboBoxSource->clear();
    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(act);
}
void DialogThreshold::activatedComboBoxThresholdMethod(int act)
{
    method = act;
}

void DialogThreshold::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}


//
// 사용자가 설정한 값으로 Threshold를 실행
//
int DialogThreshold::ThresholdRange(IplImage* grayImg, IplImage* outImg)
{
    QString str;

    int nThresholdLowValue = low;
    int nThresholdHighValue = high;
    int bInvert = 0;

    if (nThresholdHighValue == 0 && nThresholdLowValue == 0)
        return -1;

    cvInRangeS(grayImg, cv::Scalar(nThresholdLowValue), cv::Scalar(nThresholdHighValue), outImg);


    return 0;
}

//
// OTUS 알고리즘을 이용하여 히스토그램 Threshold를 실행
//
double DialogThreshold::ThresholdOTSU(IplImage* grayImg, IplImage* outImg)
{
    QString str;

    //int nThresholdValue = 70;
    int nThresholdMaxVal = 255;
    //int bInvert = 0;
    int nInvert = CV_THRESH_BINARY | CV_THRESH_OTSU;
    //if (bInvert == 1)
    //	nInvert = CV_THRESH_BINARY_INV | CV_THRESH_OTSU;
    double otsuThreshold = cvThreshold(grayImg, outImg, 0, nThresholdMaxVal, nInvert);

    return otsuThreshold;
}

int DialogThreshold::AdaptiveThreshold(IplImage* grayImg, IplImage* outImg)
{
    QString str;
    int nBlkSize = WinSize;//Local variable binarize window size
    int C = AreaSize;//Local variable binarize threshold

    int cx = grayImg->width;
    int cy = grayImg->height;

    static cv::Mat dst;
    //세번째 255는 최대값.  이렇게 하면 0과 255로 이루어진 영상으로 됨
    //마지막에서 두번째값은 threshold 계산 때 주변 픽셀 사용하는 크기.  3,5,7 식으로 홀수로 넣어줌
    //마지막은 Constant subtracted from mean or weighted mean. It may be negative

    //if (nBlkSize == 0)
    //	nBlkSize = 51;
    //if (C == 0)
    //	C = 11;
    if (nBlkSize > min(cx, cy) / 4)
        nBlkSize = min(cx, cy) / 4;
    if (nBlkSize % 2 == 0) // 강제로 홀수로 만든다.
        nBlkSize++;
    cv::Mat m1 = cv::cvarrToMat(grayImg); //cv::Mat(s)
    cv::adaptiveThreshold(m1, dst, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, nBlkSize, C);

    IplImage iplImage = dst;
    cvCopy(&iplImage, outImg);

    return 0;
}

//
// bRealtime 처리..
//
void DialogThreshold::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogThreshold::on_pushButtonExec_clicked()
{
    IplImage* iplImg = nullptr;
    if (source == 0)
    {
        ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
        if (viewMain) {
            iplImg = viewMain->getIplgray();
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
                    ExecThreshold(iplImg);
                    return;
                }

                IplImage* croppedImage;
                QRectF rect = pData->bounds();	// Area로 등록된 ROI.
                rect.normalized();

                if (rect.left() < 0)	rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= iplImg->width) rect.setRight(iplImg->width);
                if (rect.bottom() >= iplImg->height) rect.setBottom(iplImg->height);
                pData->setBounds(rect);

                cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
                cvSetImageROI(iplImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), iplImg->depth, iplImg->nChannels);
                cvCopy(iplImg, croppedImage);
                cvResetImageROI(iplImg);

                ExecThreshold(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecThreshold(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecThreshold(iplImg);
        }
    }
}

void DialogThreshold::AverageBrightInspection(IplImage* iplImg, IplImage* outImg)
{

    //cvCopy(iplImg, outImg);
    cvZero(outImg);

    double low = ui->editAvgBr1->text().toDouble();
    double high = ui->editAvgBr2->text().toDouble();

    cv::Mat greyImg = cv::cvarrToMat(iplImg);

    cv::MatND histogram;
    const int channel_numbers[1] = { 0 };
    float channel_range[2] = { 0.0, 255.0 };
    const float* ranges[1] = { channel_range } ;
    int histSize[1] = { 256 }; // number of bins

    cv::calcHist(&greyImg, 1, channel_numbers, Mat(), histogram, 1, histSize, ranges);

    // Histogram의 상하 25%를 구합니다.
    //Lower Cutoff Value
    double lowtotalcount=0;
    double uppertotalcount=0;
    int lower_cut=0;
    int upper_cut=255;
    double rangeArea = iplImg->width * iplImg->height;
    for(int iv=0; iv < histSize[0]; iv++){
        lowtotalcount += histogram.at<float>(iv);

        if(lowtotalcount >= rangeArea * 0.25) //25% under off
        {
            lower_cut=iv;
            break;
        }
    }
    uchar *imageData;
    int imgStep;

    //High Cutoff Value
    for(int iv=histSize[0]-1; iv > 0; iv--){
        uppertotalcount += histogram.at<float>(iv);
        if(uppertotalcount >= rangeArea * 0.25) //25% under off
        {
            upper_cut=iv;
            break;
        }
    }

#if 0
    // Plot the histogram
    int hist_w = outImg->width;
    int hist_h = outImg->height;
    int bin_w = cvRound((double)hist_w / histSize[0]);

    Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(0, 0, 0));
    normalize(histogram, histogram, 0, histImage.rows, NORM_MINMAX, -1, Mat());

    for (int i = 1; i < histSize[0]; i++)
    {
        cvLine(outImg, Point(bin_w*(i - 1), hist_h - cvRound(histogram.at<float>(i - 1))),
            Point(bin_w*(i), hist_h - cvRound(histogram.at<float>(i))),
            Scalar(255, 0, 0), 2, 8, 0);
    }
#endif

    //다시 상하 25% 범위의 Histogram을 구합니다.. - 상하 잡음들 제거 목적.
    MatND histogram1;
    channel_range[0] = lower_cut;
    channel_range[1] = upper_cut;
    histSize[0] = (upper_cut - lower_cut) + 1;
    calcHist(&greyImg, 1, channel_numbers, Mat(), histogram1, 1, histSize, ranges);

    double m = (greyImg.rows*greyImg.cols) / 2;
    m -= lowtotalcount;
    int bin = 0;
    double med = -1.0;
    for ( int i = 0; i < histSize[0] && med < 0.0; ++i )
    {
        bin += cvRound( histogram1.at< float >( i ) ); // 정수형으로 변환할때 반올림을 한다.
        if ( bin > m && med < 0.0 )
            med = i;
    }
    med += lower_cut; // 평균위치를 구함.

    /*
    ex) 평균값(med) 50 ( 0 ~ 255 )
    10 ~ 20% : 5 ~ 10 값 추출
    100 ~ 120% : 50 ~ 140 값 추출
    */

    // 평균값을 기준으로 범위를 설정.
    // 0 ~ 255 범위를 나타낼려다보니 아래와 같은 수식이 나옴.
    // 0 ~ 255 또는 평균값*2 ~ 255까지 나타낼수 있음.
    double dBinarizeMax;
    double dBinarize = (double)med * low/100;
    if(high > 100) {
        dBinarizeMax = (double)med + (double)(255 - med)/100*(high - 100);
    } else {
        dBinarizeMax = (double)med * high / 100;
    }

    //double areaTotal = 0; // area 계산..
    imageData = (uchar*)iplImg->imageData;
    imgStep = iplImg->widthStep;
    for (int i = 0; i < iplImg->height; i++) {
        for (int j = 0; j < iplImg->width; j++)
        {
            if(imageData[imgStep * i + j] >= dBinarize && imageData[imgStep * i + j] <= dBinarizeMax) {
                imageData[imgStep * i + j] = 255;
                //areaTotal++;
            } else {
                imageData[imgStep * i + j] = 0;
            }
        }
    }
    //cvConvertImage(iplImg, outImg,  CV_GRAY2RGB);
    cvCopy(iplImg, outImg);

#if 0
    // Plot the histogram
    hist_w = outImg->width;
    hist_h = outImg->height;
    bin_w = cvRound((double)hist_w / histSize[0]);

    normalize(histogram1, histogram1, 0, greyImg.rows, NORM_MINMAX, -1, Mat());
    for (int i = 1; i < histSize[0]; i++)
    {
        cvLine(outImg, Point(bin_w*(i - 1), hist_h - cvRound(histogram1.at<float>(i - 1))),
            Point(bin_w*(i), hist_h - cvRound(histogram1.at<float>(i))),
            Scalar(0, 255, 0), 2, 8, 0);
    }
#endif

}


void DialogThreshold::ExecThreshold(IplImage* iplImg)
{
    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);

    if (ui->chkBoxGaussian) {
       cvSmooth(iplImg, iplImg, CV_GAUSSIAN,7,7);
    }

    switch(method)
    {
        case 0:
            AdaptiveThreshold(iplImg, tmp);
            break;
        case 1:
            ThresholdRange(iplImg, tmp);
            break;
        case 2:
            ThresholdOTSU(iplImg, tmp);
            break;
        case 3:
            cvEqualizeHist(iplImg, tmp);
            break;
        case 4:
            AverageBrightInspection(iplImg, tmp);
            break;
    }
    if (bNot)
        cvNot(tmp, tmp);

    theMainWindow->outWidget(mName, tmp);
}
