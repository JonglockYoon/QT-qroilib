#include "dialoglinefit.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialoglinefit.h"

struct SLine
{
    SLine():
        numOfValidPoints(0),
        params(-1.f, -1.f, -1.f, -1.f)
    {}
    cv::Vec4f params;//(cos(t), sin(t), X0, Y0)
    int numOfValidPoints;
};

DialogLinefit::DialogLinefit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLinefit)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("RansacLinefit%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    ui->setupUi(this);

    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(0);



    connect(ui->comboBoxSource, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));
}

DialogLinefit::~DialogLinefit()
{
}

void DialogLinefit::closeEvent(QCloseEvent *event)
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

void DialogLinefit::changeRealtime(bool checked)
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

void DialogLinefit::activatedComboBoxSource(int act)
{
    source = act;
}

void DialogLinefit::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}

//
// bRealtime 처리..
//
void DialogLinefit::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogLinefit::on_pushButtonExec_clicked()
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
                    ExecRansacLinefit(iplImg);
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

                ExecRansacLinefit(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecRansacLinefit(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecRansacLinefit(iplImg);
        }
    }
}

void DialogLinefit::ExecRansacLinefit(IplImage* iplImg)
{
    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);
    //cvZero(tmp);

    cvInRangeS(iplImg, cv::Scalar(100), cv::Scalar(255), iplImg);

//     IplConvKernel *element = nullptr;
//     int filterSize = 3;  // 필터의 크기를 3으로 설정 (Noise out area)
//     element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);
//     cvMorphologyEx(iplImg, iplImg, nullptr, element, CV_MOP_OPEN, 7);
//     cvReleaseStructuringElement(&element);

//     element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);
//     cvMorphologyEx(iplImg, iplImg, nullptr, element, CV_MOP_GRADIENT, 3);
//     cvReleaseStructuringElement(&element);

//     element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);
//     cvMorphologyEx(iplImg, iplImg, nullptr, element, CV_MOP_ERODE, 2);
//     cvReleaseStructuringElement(&element);


    cvSmooth(iplImg, iplImg, CV_GAUSSIAN,3,3);
    cvCopy(iplImg, tmp);

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(iplImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);




    float line[4];
    while(contours)
    {
//        if (contours->total < 10)
//        {
//            contours = contours->h_next;
//            continue;
//        }
        // CV_DIST_L2 : 거리 유형
        // 0 : L2 거리 사용하지 않음
        // 0.01 : 정확도
        cvFitLine(contours, CV_DIST_HUBER, 0, 0.001, 0.001, line);
        CvRect boundbox = cvBoundingRect(contours);
        int xlen = boundbox.width / 2;
        int ylen = boundbox.height / 2;


        double d = sqrt(line[0] * line[0] + line[1] * line[1]);
        line[0] /= d;
        line[1] /= d;
        double t = boundbox.width + boundbox.height;

        // 올바른 선을 계산하는지 확인하기 위해 영상에 예상 선을 그림
        int x0= line[2]; // 선에 놓은 한 점
        int y0= line[3];
        int x1= x0 - t*line[0]; // 기울기에 길이를 갖는 벡터 추가
        int y1= y0 - t*line[1];
        int x2= x0 + t*line[0];
        int y2= y0 + t*line[1];
        cvLine( tmp, CvPoint(x1,y1), CvPoint(x2,y2), CV_RGB(128,128,128), 1, 8 );

        contours = contours->h_next;
    }

    cvReleaseMemStorage(&storage);

    theMainWindow->outWidget(mName, tmp);
}

