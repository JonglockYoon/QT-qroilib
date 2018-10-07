#include "dialogmorphology.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogmorphology.h"

DialogMorphology::DialogMorphology(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMorphology)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Morphology%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    method = 0;
    shape = CV_SHAPE_RECT;

    ui->setupUi(this);

    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(0);


    // Half size
    ui->slidera1->setRange(0, 20);
    ui->slidera1->setValue(HalfSize);
    ui->slidera1->setSingleStep(1);;

    connect(ui->comboBoxSource, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));

    // Half size
    QObject::connect(ui->slidera1, SIGNAL(valueChanged(int)), this, SLOT(setValuea1(int)));
    QObject::connect(ui->edita1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuea1(const QString &)));
    str = QString("%1").arg(HalfSize);
    ui->edita1->setText(str);


    ui->radioButtonErode->setChecked(true);
    ui->radioButtonRect->setChecked(true);
}

DialogMorphology::~DialogMorphology()
{
}

void DialogMorphology::closeEvent(QCloseEvent *event)
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

void DialogMorphology::setValuea1(int val)
{
    HalfSize = val;
    QString str = QString("%1").arg(val);
    ui->edita1->setText(str);
}

void DialogMorphology::setEditValuea1(const QString &val)
{
    int pos = val.toInt();
    ui->slidera1->setValue(pos);
}

void DialogMorphology::activatedComboBoxSource(int act)
{
    source = act;

    ui->comboBoxSource->clear();
    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(act);}


void DialogMorphology::on_radioButtonErode_clicked()
{
    method = 0;
}

void DialogMorphology::on_radioButtonDilate_clicked()
{
    method = 1;
}

void DialogMorphology::on_radioButtonOpen_clicked()
{
    method = 2;
}

void DialogMorphology::on_radioButtonClose_clicked()
{
    method = 3;
}

void DialogMorphology::on_radioButtonTophat_clicked()
{
    method = 4;
}

void DialogMorphology::on_radioButtonBlackhat_clicked()
{
    method = 5;
}

void DialogMorphology::on_radioButtonGradient_clicked()
{
    method = 6;
}

void DialogMorphology::on_radioButtonMedian_clicked()
{
    method = 7;
}

void DialogMorphology::changeRealtime(bool checked)
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

void DialogMorphology::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}

//
// bRealtime 처리..
//
void DialogMorphology::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogMorphology::on_pushButtonExec_clicked()
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
                    ExecMorphology(iplImg);
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

                ExecMorphology(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecMorphology(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecMorphology(iplImg);
        }
    }

}

void DialogMorphology::ExecMorphology(IplImage* iplImg)
{
    IplConvKernel *element = nullptr;
    int filterSize = 3;  // 필터의 크기를 3으로 설정 (Noise out area)

    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);
    if (filterSize <= 0)
        filterSize = 1;
    if (filterSize % 2 == 0)
        filterSize++;

    switch(method)
    {
        case 0: // Erode
            cvErode(iplImg, tmp, nullptr, HalfSize);
            break;
        case 1: // Dilate
            cvDilate(iplImg, tmp, nullptr, HalfSize);
            break;
        case 2: // Open
            element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, shape, nullptr);
            cvMorphologyEx(iplImg, tmp, nullptr, element, CV_MOP_OPEN, HalfSize);
            cvReleaseStructuringElement(&element);
            break;
        case 3: // Close
            element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, shape, nullptr);
            cvMorphologyEx(iplImg, tmp, nullptr, element, CV_MOP_CLOSE, HalfSize);
            cvReleaseStructuringElement(&element);
            break;
        case 4: // Top hat
            element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, shape, nullptr);
            cvMorphologyEx(iplImg, tmp, nullptr, element, CV_MOP_TOPHAT, HalfSize);
            cvReleaseStructuringElement(&element);
            break;
        case 5: // Black hat
            element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, shape, nullptr);
            cvMorphologyEx(iplImg, tmp, nullptr, element, CV_MOP_BLACKHAT, HalfSize);
            cvReleaseStructuringElement(&element);
            break;
        case 6: // Gradient
            element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, shape, nullptr);
            cvMorphologyEx(iplImg, tmp, nullptr, element, CV_MOP_GRADIENT, HalfSize);
            cvReleaseStructuringElement(&element);
            break;
        case 7: // Median
            if (HalfSize%2==0)
                HalfSize++;
            cvSmooth(iplImg, tmp, CV_MEDIAN, HalfSize);
            break;
    }

    theMainWindow->outWidget(mName, tmp);
}

void DialogMorphology::on_radioButtonRect_clicked()
{
    shape = CV_SHAPE_RECT;
}

void DialogMorphology::on_radioButtonEllipse_clicked()
{
    shape = CV_SHAPE_ELLIPSE;
}

void DialogMorphology::on_radioButtonCross_clicked()
{
    shape = CV_SHAPE_CROSS;
}
