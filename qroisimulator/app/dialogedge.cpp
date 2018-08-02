#include "dialogedge.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogedge.h"

//
// cvCanny()
//   threshold1:상단임계값-픽셀의 그래디언트 크기가 상단 임계값보다 크면 엣지 픽셀로 간주.
//   threshold2:하단임계값-픽셀의 그래디언트 크기가 하단 임계값보다 작으면 엣지에서 제외.
//   그래디언트 값이 두 임계값 사이에 존재하는경우에는, 이픽셀 주변에 엣지 픽셀이 존재하는경우에만 엣지로 간주.
//
//

DialogEdge::DialogEdge(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEdge)
{
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Edge%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    method = 0;
    nGaussian = 3;
    bSmooth = false;

    ui->setupUi(this);

    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(0);

    ui->comboBoxEdgeMethod->insertItem(0, QIcon(), QString::fromLocal8Bit("Canny"));
    ui->comboBoxEdgeMethod->insertItem(1, QIcon(), QString::fromLocal8Bit("Sobel"));
    ui->comboBoxEdgeMethod->insertItem(2, QIcon(), QString::fromLocal8Bit("Laplacian"));
    ui->comboBoxEdgeMethod->setCurrentIndex(method);

    QString str = QString("%1").arg(nGaussian);
    ui->editGaussian->setText(str);

    // Canny group
    ui->slider1->setRange(0, 500);
    ui->slider1->setValue(val1);
    ui->slider1->setSingleStep(1);
    ui->slider2->setRange(0, 500);
    ui->slider2->setValue(val2);
    ui->slider2->setSingleStep(1);


    connect(ui->comboBoxSource, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource(int)));
    connect(ui->comboBoxEdgeMethod, SIGNAL(activated(int)), this, SLOT(activatedComboBoxEdgeMethod(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));

    // Canny group
    QObject::connect(ui->slider1, SIGNAL(valueChanged(int)), this, SLOT(setValue1(int)));
    QObject::connect(ui->edit1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValue1(const QString &)));
    QObject::connect(ui->slider2, SIGNAL(valueChanged(int)), this, SLOT(setValue2(int)));
    QObject::connect(ui->edit2, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValue2(const QString &)));
    QObject::connect(ui->chkBoxSmooth, SIGNAL(clicked(bool)), this, SLOT(changeSmooth(bool)));
    QObject::connect(ui->editGaussian, SIGNAL(textChanged(const QString &)), this, SLOT(setEditGaussian(const QString &)));
    str = QString("%1").arg(val1);
    ui->edit1->setText(str);
    str = QString("%1").arg(val2);
    ui->edit2->setText(str);

    // Sobel group
    QObject::connect(ui->editSobelX, SIGNAL(textChanged(const QString &)), this, SLOT(setSobelEditValueX(const QString &)));
    QObject::connect(ui->editSobelY, SIGNAL(textChanged(const QString &)), this, SLOT(setSobelEditValueY(const QString &)));
    QObject::connect(ui->chkBoxSobelWithSign, SIGNAL(clicked(bool)), this, SLOT(changeWithSign(bool)));
    str = QString("%1").arg(sobelX);
    ui->editSobelX->setText(str);
    str = QString("%1").arg(sobelY);
    ui->editSobelY->setText(str);

    // Laplacian group
    QObject::connect(ui->editAptSize, SIGNAL(textChanged(const QString &)), this, SLOT(seteditAptSizeValue(const QString &)));
    str = QString("%1").arg(aperture_size);
    ui->editAptSize->setText(str);
}

DialogEdge::~DialogEdge()
{
}

void DialogEdge::closeEvent(QCloseEvent *event)
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

void DialogEdge::setValue1(int val)
{
    val1 = val;
    QString str = QString("%1").arg(val);
    ui->edit1->setText(str);
}
void DialogEdge::setValue2(int val)
{
    val2 = val;
    QString str = QString("%1").arg(val);
    ui->edit2->setText(str);
}

void DialogEdge::SobelNormalize(IplImage *sobel, IplImage *image, int x, int y)
{
   double min_val,max_val,diff, scale;

   if (x == 1 && y == 0)
   {
       //sobel x
       cvMinMaxLoc( sobel, &min_val, &max_val);  //글로벌 미니멈 이랑 맥시멈 찾는다.
       qDebug() << "max val : " << max_val << "min val : " << min_val;
       diff=fabs(max_val)-fabs(min_val);

       if (diff<0)
       {
           scale=255/fabs(min_val);
       }
       else
       {
           scale=255/fabs(max_val);
       }

       if (!bWithSign)
            cvConvertScaleAbs( sobel, image, scale );  // edge의 부호는 고려치 않음
       else cvConvertScaleAbs( sobel, image, 0.5*scale, 127 );   // edge의 부호도 고려
   }
   else if (x == 0 && y == 1)
   {
        //sobel y
        cvMinMaxLoc( sobel, &min_val, &max_val);  //글로벌 미니멈 이랑 맥시멈 찾는다.
        qDebug() << "max val : " << max_val << "min val : " << min_val;
        diff=fabs(max_val)-fabs(min_val);

        if (diff<0)
        {
         scale=255/fabs(min_val);
        }
        else
        {
         scale=255/fabs(max_val);
        }

        if (!bWithSign)
            cvConvertScaleAbs( sobel, image, scale );// edge의 부호는 고려치 않음
        else cvConvertScaleAbs( sobel, image, 0.5*scale, 127 ); // edge의 부호도 고려
    }
   else
   {
        //sobel xy
        cvMinMaxLoc( sobel, &min_val, &max_val);  //글로벌 미니멈 이랑 맥시멈 찾는다.
        qDebug() << "max val : " << max_val << "min val : " << min_val;
        diff=fabs(max_val)-fabs(min_val);

        if (diff<0)
        {
         scale=255/fabs(min_val);
        }
        else
        {
         scale=255/fabs(max_val);
        }

        if (!bWithSign)
            cvConvertScaleAbs( sobel, image, scale );// edge의 부호는 고려치 않음
        else cvConvertScaleAbs( sobel, image, 0.5*scale, 127 ); // edge의 부호도 고려
   }
}

void DialogEdge::setEditValue1(const QString &val)
{
    int pos = val.toInt();
    ui->slider1->setValue(pos);
}
void DialogEdge::setEditValue2(const QString &val)
{
    int pos = val.toInt();
    ui->slider2->setValue(pos);
}

void DialogEdge::setSobelEditValueX(const QString &val)
{
    sobelX = val.toInt();
}
void DialogEdge::setSobelEditValueY(const QString &val)
{
    sobelY = val.toInt();
}
void DialogEdge::seteditAptSizeValue(int val)
{
    aperture_size = val;
}

void DialogEdge::changeRealtime(bool checked)
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
void DialogEdge::changeSmooth(bool checked)
{
    bSmooth = checked;
}
void DialogEdge::changeWithSign(bool checked)
{
    bWithSign = checked;
}

void DialogEdge::setEditGaussian(const QString &val)
{
    nGaussian = val.toInt();
}

void DialogEdge::activatedComboBoxSource(int act)
{
    source = act;
}
void DialogEdge::activatedComboBoxEdgeMethod(int act)
{
    method = act;
}

void DialogEdge::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}

//
// bRealtime 처리..
//
void DialogEdge::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogEdge::on_pushButtonExec_clicked()
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
                    ExecEdge(iplImg);
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

                ExecEdge(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecEdge(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
        }
        ExecEdge(iplImg);
    }
}
void DialogEdge::ExecEdge(IplImage* iplImg)
{
    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }
    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);

    if (bSmooth) {
        try {
            cvSmooth(iplImg, iplImg, CV_GAUSSIAN,nGaussian,nGaussian);
        } catch (...) {
            qDebug() << "Error cvSmooth()";
            return;
        }
    }

    switch(method)
    {
        case 0:
            cvCanny(iplImg, tmp, val1, val2, 3);
            break;
        case 1:
            if (sobelX == 0 && sobelX == 0)
                break;
            sobel_tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), IPL_DEPTH_16S, 1);
            cvSobel(iplImg, sobel_tmp, sobelX, sobelX);
            SobelNormalize(sobel_tmp, tmp, sobelX, sobelX);
            cvReleaseImage(&sobel_tmp);
            break;
        case 2:
            if (aperture_size%2 == 0)
                aperture_size++;
            cvLaplace(iplImg, tmp, aperture_size);
            break;
    }

    theMainWindow->outWidget(mName, tmp);
}
