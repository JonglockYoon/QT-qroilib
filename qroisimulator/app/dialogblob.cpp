#include "dialogblob.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogblob.h"

DialogBlob::DialogBlob(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBlob)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Blob%1%2").arg(date).arg(time);
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
    QObject::connect(ui->chkBoxThinner, SIGNAL(clicked(bool)), this, SLOT(changeThinner(bool)));


    Noiseout1 = 0;
    Noiseout2 = 0;
    Noiseout3 = 0;
    Noiseout4 = 0;
    QObject::connect(ui->edit1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValue1(const QString &)));
    QObject::connect(ui->edit2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValue2(const QString &)));
    QObject::connect(ui->edit3, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValue3(const QString &)));
    QObject::connect(ui->edit4, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValue4(const QString &)));
    str = QString("%1").arg(Noiseout1);
    ui->edit1->setText(str);
    str = QString("%1").arg(Noiseout2);
    ui->edit2->setText(str);
    str = QString("%1").arg(Noiseout3);
    ui->edit3->setText(str);
    str = QString("%1").arg(Noiseout4);
    ui->edit4->setText(str);

    MinArea = 0;
    MaxArea = 0;
    QObject::connect(ui->edita1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuea1(const QString &)));
    QObject::connect(ui->edita2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuea2(const QString &)));
    str = QString("%1").arg(MinArea);
    ui->edita1->setText(str);
    str = QString("%1").arg(MaxArea);
    ui->edita2->setText(str);

    MinX = 0;
    MaxX = 0;
    QObject::connect(ui->editx1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuex1(const QString &)));
    QObject::connect(ui->editx2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuex2(const QString &)));
    str = QString("%1").arg(MinX);
    ui->editx1->setText(str);
    str = QString("%1").arg(MaxX);
    ui->editx2->setText(str);
    MinY = 0;
    MaxY = 0;
    QObject::connect(ui->edity1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuey1(const QString &)));
    QObject::connect(ui->edity2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuey2(const QString &)));
    str = QString("%1").arg(MinY);
    ui->edity1->setText(str);
    str = QString("%1").arg(MaxY);
    ui->edity2->setText(str);
}

DialogBlob::~DialogBlob()
{
}

void DialogBlob::closeEvent(QCloseEvent *event)
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

void DialogBlob::changeThinner(bool checked)
{
    bThinner = checked;
}

void DialogBlob::changeRealtime(bool checked)
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

void DialogBlob::activatedComboBoxSource(int act)
{
    source = act;
}

void DialogBlob::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}
void DialogBlob::setEditValue1(const QString &val)
{
    Noiseout1 = val.toInt();
}
void DialogBlob::setEditValue2(const QString &val)
{
    Noiseout2 = val.toInt();
}
void DialogBlob::setEditValue3(const QString &val)
{
    Noiseout3 = val.toInt();
}
void DialogBlob::setEditValue4(const QString &val)
{
    Noiseout4 = val.toInt();
}

void DialogBlob::setEditValuea1(const QString &val)
{
    MinArea = val.toDouble();
}
void DialogBlob::setEditValuea2(const QString &val)
{
    MaxArea = val.toDouble();
}

void DialogBlob::setEditValuex1(const QString &val)
{
    MinX = val.toInt();
}
void DialogBlob::setEditValuex2(const QString &val)
{
    MaxX = val.toInt();
}
void DialogBlob::setEditValuey1(const QString &val)
{
    MinY = val.toInt();
}
void DialogBlob::setEditValuey2(const QString &val)
{
    MaxY = val.toInt();
}


//
// 모폴리지를 이용하여 잡음제거
//
int DialogBlob::NoiseOut(IplImage* grayImg)
{
    QString str;

    int filterSize = 3;
    IplConvKernel *element = nullptr;
    if (filterSize <= 0)
        filterSize = 1;
    if (filterSize % 2 == 0)
        filterSize++;
    element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);

    int nNoiseout = Noiseout1;
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = Noiseout2;
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = Noiseout3;
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = Noiseout4;
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    cvReleaseStructuringElement(&element);
    return 0;
}


void DialogBlob::FilterArea(IplImage* grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        double area = p->Area();
        if (MaxArea > 0 && area > MaxArea)
            p->ClearContours();
        if (area < MinArea)
            p->ClearContours();
    }
    cvZero(grayImg);
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
    }
}

void DialogBlob::FilterDiameter(IplImage* grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        CvRect r = p->GetBoundingBox();
        if ((MaxY > 0 && r.height > MaxY) || (MaxX > 0 && r.width > MaxX))
            p->ClearContours();
        if (r.height < MinY || r.width < MinX)
            p->ClearContours();
    }
    cvZero(grayImg);
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
    }
}


//
// 입력된 흑백 이미지 Thinner 처리
//
void DialogBlob::Thinner(IplImage* grayImg)
{
    cv::Mat1b m = cv::Mat1b(cv::cvarrToMat(grayImg).clone());

    IplImage *trans = nullptr;
    bool ok = thinner.thin(m, IMPL_GUO_HALL_FAST, false);
    if (ok) {
        IplImage iplImage = thinner.get_skeleton();
        trans = &iplImage;
    }

    if (trans)
        cvCopy(trans, grayImg);
    else
        cvZero(grayImg);
    m.release();
}

//
// bRealtime 처리..
//
void DialogBlob::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogBlob::on_pushButtonExec_clicked()
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
                    ExecBlob(iplImg);
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

                ExecBlob(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecBlob(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecBlob(iplImg);
        }
    }
}

void DialogBlob::ExecBlob(IplImage* iplImg)
{
    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);

    cvCopy(iplImg, tmp);
    NoiseOut(tmp);
    FilterArea(tmp);
    FilterDiameter(tmp);
    if (bThinner)
        Thinner(tmp);

    theMainWindow->outWidget(mName, tmp);
}
