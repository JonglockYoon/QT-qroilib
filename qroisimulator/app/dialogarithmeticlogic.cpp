#include "dialogarithmeticlogic.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogarithmeticlogic.h"

DialogArithmeticlogic::DialogArithmeticlogic(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogArithmeticlogic)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Arithmeticlogic%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    ui->setupUi(this);

    source0 = 0;
    source1 = 0;
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

    ui->radioButtonCopy->setChecked(true);

    connect(ui->comboBoxSource0, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource0(int)));
    connect(ui->comboBoxSource1, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource1(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));
}

DialogArithmeticlogic::~DialogArithmeticlogic()
{
}

void DialogArithmeticlogic::closeEvent(QCloseEvent *event)
{
//    if (bRealtime)
//    {
//        if (source0 == 0)
//        {
//            ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
//            if (pView) {
//                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//            }
//        } else {
//            OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
//            ViewOutPage* pView = pWidget->mViewOutPage;
//            if (pView) {
//                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//            }
//        }
//    }

    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
}

void DialogArithmeticlogic::changeRealtime(bool checked)
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

void DialogArithmeticlogic::activatedComboBoxSource0(int act)
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
void DialogArithmeticlogic::activatedComboBoxSource1(int act)
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

void DialogArithmeticlogic::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}

//
// bRealtime 처리..
//
void DialogArithmeticlogic::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogArithmeticlogic::on_pushButtonExec_clicked()
{
    IplImage* iplImg = nullptr;
    if (source0 == 0)
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
                    ExecArithmeticlogic(iplImg);
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

                ExecArithmeticlogic(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecArithmeticlogic(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecArithmeticlogic(iplImg);
        }
    }
}

void DialogArithmeticlogic::ExecArithmeticlogic(IplImage* iplImg)
{
    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);

    IplImage* iplImg1 = nullptr;
    int size = theMainWindow->vecOutWidget.size();
    if (size > 0)
    {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg1 = pView->getIplgray();
        }
    }

    switch (method) {
    case 0: // Copy
        cvCopy(iplImg, tmp);
        break;
    case 1: // Invert
        cvNot(iplImg, tmp);
        break;
    case 2: // Add
        if (iplImg1 == nullptr)
            return;
        cvAdd(iplImg, iplImg1, tmp);
        break;
    case 3: // Subtract
        if (iplImg1 == nullptr)
            return;
        cvSub(iplImg, iplImg1, tmp);
        break;
    case 4: // AbsDiff
        if (iplImg1 == nullptr)
            return;
        cvAbsDiff(iplImg, iplImg1, tmp);
        break;
    case 5: // And
        if (iplImg1 == nullptr)
            return;
        cvAnd(iplImg, iplImg1, tmp);
        break;
    case 6: // Or
        if (iplImg1 == nullptr)
            return;
        cvOr(iplImg, iplImg1, tmp);
        break;
    case 7: // XOr
        if (iplImg1 == nullptr)
            return;
        cvXor(iplImg, iplImg1, tmp);
        break;
    }




    theMainWindow->outWidget(mName, tmp);
}

void DialogArithmeticlogic::on_radioButtonCopy_clicked()
{
    method = 0;
}

void DialogArithmeticlogic::on_radioButtonInvert_clicked()
{
    method = 1;
}

void DialogArithmeticlogic::on_radioButtonAdd_clicked()
{
    method = 2;
}

void DialogArithmeticlogic::on_radioButtonSubtract_clicked()
{
    method = 3;
}

void DialogArithmeticlogic::on_radioButtonAbsDiff_clicked()
{
    method = 4;
}

void DialogArithmeticlogic::on_radioButtonAnd_clicked()
{
    method = 5;
}

void DialogArithmeticlogic::on_radioButtonOr_clicked()
{
    method = 6;
}

void DialogArithmeticlogic::on_radioButtonXor_clicked()
{
    method = 7;
}
