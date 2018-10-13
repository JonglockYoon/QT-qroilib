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

    // threshold group
    QObject::connect(ui->sliderth1, SIGNAL(valueChanged(int)), this, SLOT(setValueth1(int)));
    QObject::connect(ui->sliderth2, SIGNAL(valueChanged(int)), this, SLOT(setValueth2(int)));
    QObject::connect(ui->editth1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValueth1(const QString &)));
    QObject::connect(ui->editth2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValueth2(const QString &)));
    str = QString("%1").arg(low);
    ui->editth1->setText(str);
    str = QString("%1").arg(high);
    ui->editth2->setText(str);

    connect(ui->comboBoxSource, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));

    ui->radioButtonPoints->setChecked(true);

    rng = cvRNG(-1);

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

    ui->comboBoxSource->clear();
    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(act);
}

void DialogLinefit::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}


void DialogLinefit::setValueth1(int val)
{
    low = val;
    QString str = QString("%1").arg(val);
    ui->editth1->setText(str);
}
void DialogLinefit::setValueth2(int val)
{
    high = val;
    QString str = QString("%1").arg(val);
    ui->editth2->setText(str);
}
void DialogLinefit::setEditValueth1(const QString &val)
{
    low = val.toInt();
    ui->sliderth1->setValue(low);
}
void DialogLinefit::setEditValueth2(const QString &val)
{
    high = val.toInt();
    ui->sliderth2->setValue(high);
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

void DialogLinefit::ExecRansacLinefitPoints(IplImage* iplImg)
{
    cvCopy(iplImg, tmp);

    CBlobResult blobs;
    blobs = CBlobResult(iplImg, nullptr);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount <= 0)
        return;

    CvPoint* points = new CvPoint[blobCount];

    for (int i = 0; i < blobCount; i++) // 각 점들의 중심점을 구합니다.
    {
        CBlob *currentBlob = blobs.GetBlob(i);
        double m00 = currentBlob->Moment(0,0);
        double m01 = currentBlob->Moment(0,1);
        double m10 = currentBlob->Moment(1,0);

        CvPoint p(m10/m00, m01/m00);
        if (p.x < 0 || p.y < 0)
            continue;
        points[i].x = p.x;
        points[i].y = p.y;
    }

    //cvFitLine으로 최적의 직선을 찾습니다.
    CvMat pointMat = cvMat(1, blobCount, CV_32SC2, points);
    float line[4];
    cvFitLine(&pointMat, CV_DIST_L1, 1, 0.001, 0.001, line);

    float d,t;

    d = sqrt(line[0] * line[0] + line[1] * line[1]);
    line[0] /= d;
    line[1] /= d;
    t = tmp->width + tmp->height;

    // 영상에 선을 그립니다.
    int x0= line[2]; // 선에 놓은 한 점
    int y0= line[3];
    int x1= x0 - t*line[0]; // 기울기에 길이를 갖는 벡터 추가
    int y1= y0 - t*line[1];
    int x2= x0 + t*line[0];
    int y2= y0 + t*line[1];
    cvLine( tmp, CvPoint(x1,y1), CvPoint(x2,y2), CV_RGB(128,128,128), 1, 8 );

    delete points;
}
void DialogLinefit::ExecRansacLinefitBlob(IplImage* iplImg)
{

    CImgProcBase base;
    base.FilterLargeArea(iplImg);

    cvSmooth(iplImg, iplImg, CV_GAUSSIAN,3,3);
    cvCopy(iplImg, tmp);

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(iplImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    float line[4];
    while(contours)
    {
        // CV_DIST_HUBER : Distance types for Distance Transform and M-estimators
        // 0 :
        // 0.01 : 정확도
        cvFitLine(contours, CV_DIST_HUBER, 0, 0.001, 0.001, line);
        CvRect boundbox = cvBoundingRect(contours);

        double d = sqrt(line[0] * line[0] + line[1] * line[1]);
        line[0] /= d;
        line[1] /= d;
        double t = boundbox.width + boundbox.height;

        // 영상에 선을 그립니다.
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
    cvZero(tmp);

    cvInRangeS(iplImg, cv::Scalar(low), cv::Scalar(high), iplImg);


    if (ui->radioButtonPoints->isChecked())
        ExecRansacLinefitPoints(iplImg);
    if (ui->radioButtonBlob->isChecked())
        ExecRansacLinefitBlob(iplImg);

    theMainWindow->outWidget(mName, tmp);
}

