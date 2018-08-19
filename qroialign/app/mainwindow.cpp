/*

QROILIB : QT Vision ROI Library
Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux 및 windows에서 test되었다.

------

qroialign:
    Image align sample program

*/

#include <roilib_export.h>
// Qt
#include <QApplication>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QMenuBar>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>

// Local
#include <qroilib/gvdebug.h>
#include "mainwindow.h"
#include "common.h"
#include "recipedata.h"

struct MainWindow::Private
{
    MainWindow* q;
    QWidget* mContentWidget;
    ViewMainPage* mViewMainPage;
    QStackedWidget* mViewStackedWidget;
    QActionGroup* mViewModeActionGroup;

    QMenu *fileMenu;
    QMenu *viewMenu;

    QAction *readImage;
    QAction *exitAct;

    void setupWidgets()
    {
        mContentWidget = new QWidget(q);
        q->setCentralWidget(mContentWidget);

        mViewStackedWidget = new QStackedWidget(mContentWidget);
        QVBoxLayout* layout = new QVBoxLayout(mContentWidget);
        layout->addWidget(mViewStackedWidget);
        layout->setMargin(0);
        layout->setSpacing(0);

        setupViewMainPage(mViewStackedWidget);
        mViewStackedWidget->addWidget(mViewMainPage);

    }

    void setupViewMainPage(QWidget* parent)
    {
        mViewMainPage = new ViewMainPage(parent);
    }

    void setupActions()
    {
        readImage = new QAction(QIcon(), tr("&Read Image ..."), q);
        connect(readImage, SIGNAL(triggered(bool)), q, SLOT(setReadImage()));

        exitAct = new QAction(QIcon(), tr("E&xit"), q);
        exitAct->setShortcuts(QKeySequence::Quit);
        connect(exitAct, SIGNAL(triggered()), q, SLOT(close()));

        fileMenu = new QMenu(tr("&File"), q);

        fileMenu->addAction(readImage);
        fileMenu->addAction(exitAct);
        viewMenu = new QMenu(tr("&View"), q);

        q->menuBar()->addMenu(fileMenu);
        q->menuBar()->addMenu(viewMenu);

    }

};

MainWindow* theMainWindow = 0;

MainWindow::MainWindow()
: d(new MainWindow::Private)
{
    theMainWindow = this;

    setMinimumSize(800, 480);
    g_cRecipeData = new CRecipeData(this);

    d->q = this;
    d->setupWidgets();
    d->setupActions();
    d->mViewMainPage->loadConfig();

    extern Qroilib::ParamTable ROIDSHARED_EXPORT paramTable[];
    DocumentView* v = currentView();
    if (v)
    {
        v->setParamTable(paramTable);
        v->bMultiView = false;
        connect(v, &Qroilib::DocumentView::finishNewRoiObject, this, &MainWindow::finishNewRoiObject);
        v->selectTool(v->actSelectTool);
    }

    mLeftDock = new LeftDock(tr("Menu"), this);
    mLeftDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::LeftDockWidgetArea, mLeftDock);
    d->viewMenu->addAction(mLeftDock->toggleViewAction());

}


MainWindow::~MainWindow()
{
}

ViewMainPage* MainWindow::viewMainPage() const
{
    return d->mViewMainPage;
}

DocumentView* MainWindow::currentView() const
{
    DocumentView* v = d->mViewMainPage->currentView();
    return v;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    event->ignore();
    QString str = tr("Close application");
    qDebug() << str;

    int ret = QMessageBox::warning(this, str,
                                   tr("Do you really want to quit?"),
                                   QMessageBox::Ok| QMessageBox::Cancel,
                                   QMessageBox::Ok);
    switch(ret)
    {
        case QMessageBox::Ok:
            event->accept();

            delete d->mViewMainPage;
            delete d;

            qApp->quit(); // terminates application.
            break;
    }
}

void MainWindow::finishNewRoiObject()
{
    Qroilib::DocumentView* v = currentView();
    v->clearSelectedObjectItems();
    v->selectTool(v->actSelectTool);
}



int MainWindow::InspectOneItem(IplImage* img, RoiObject *pData)
{

    QString str;
    str.sprintf("InspectOneItem type=%d", pData->mInspectType);

    if (pData == nullptr)
        return -1;

    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++) {
        DetectResult *prst = &pData->m_vecDetectResult[i];
        if (prst->ngBlobImg)
            cvReleaseImage(&prst->ngBlobImg);
    }
    pData->m_vecDetectResult.clear();

    CvSize searchSize = cvSize(img->width, img->height);
    IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);

    if (img->nChannels == 3)
        cvCvtColor(img, graySearchImg, CV_RGB2GRAY);
    else if (img->nChannels == 4) {
        if (strncmp(img->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(img, graySearchImg, CV_BGRA2GRAY);
        else
            cvCvtColor(img, graySearchImg, CV_RGBA2GRAY);
    } else
        cvCopy(img, graySearchImg);

    IplImage* croppedImage;
    QRectF rect = pData->bounds();	// Area로 등록된 ROI.
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= graySearchImg->width) rect.setRight(graySearchImg->width);
    if (rect.bottom() >= graySearchImg->height) rect.setBottom(graySearchImg->height);
    pData->setBounds(rect);

    cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
    cvSetImageROI(graySearchImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
    croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), graySearchImg->depth, graySearchImg->nChannels);
    cvCopy(graySearchImg, croppedImage);
    cvResetImageROI(graySearchImg);

    switch (pData->mInspectType)
    {
    case _Inspect_Roi_Align_TowPoint: // corner
        align.SingleROICorner(croppedImage, pData, rect);
        DrawResultCrossMark(img, pData);
        break;
    case _Inspect_Roi_Align_Measure: // ramp line
        align.SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
        DrawResultCrossMark(img, pData);
        break;
    }

    cvReleaseImage(&croppedImage);
    cvReleaseImage(&graySearchImg);

    viewMainPage()->bPreview = false;
}


double MainWindow::SinglePattFind(IplImage* grayImage, RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    if (pData->iplTemplate == nullptr)
        return -1;
    QString str;

    CvSize searchSize = cvSize(grayImage->width, grayImage->height);
    IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
    if (grayImage->nChannels == 3)
        cvCvtColor(grayImage, graySearchImg, CV_RGB2GRAY);
    else
        cvCopy(grayImage, graySearchImg);


    CvSize templateSize = cvSize(pData->iplTemplate->width, pData->iplTemplate->height);
    IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
    if (pData->iplTemplate->nChannels == 3)
        cvCvtColor(pData->iplTemplate, grayTemplateImg, CV_RGB2GRAY);
    else
        cvCopy(pData->iplTemplate, grayTemplateImg);

    double min, max;
    CvPoint left_top;
    // 상관계수를 구할 이미지.
    IplImage *coeff = cvCreateImage( cvSize( grayImage->width - grayTemplateImg->width+1,
                             grayImage->height - grayTemplateImg->height+1 ), IPL_DEPTH_32F, 1 );
    // 상관계수를 구하여 coeff에 그려준다.
    cvMatchTemplate(grayImage, grayTemplateImg, coeff, CV_TM_CCOEFF_NORMED);
    // 상관계수가 최대값을 가지는 위치를 찾는다.
    cvMinMaxLoc(coeff, &min, &max, NULL, &left_top);

    cvReleaseImage(&coeff);
    DetectResult detectResult;
    detectResult.rect.setLeft(pData->bounds().x() + left_top.x);
    detectResult.rect.setTop(pData->bounds().y() + left_top.y);
    detectResult.rect.setRight(detectResult.rect.left() + pData->iplTemplate->width);
    detectResult.rect.setBottom(detectResult.rect.top() + pData->iplTemplate->height);
    pData->m_vecDetectResult.push_back(detectResult);

    return max;
}

void MainWindow::DrawResultCrossMark(IplImage *iplImage, RoiObject *pData)
{
    if (iplImage == nullptr) return;

    QRectF rect = pData->bounds();	// Area로 등록된 ROI.
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= iplImage->width) rect.setRight(iplImage->width);
    if (rect.bottom() >= iplImage->height) rect.setBottom(iplImage->height);

    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++) {
        DetectResult *prst = &pData->m_vecDetectResult[i];
        qDebug() << "DrawResultCrossMark" << prst->pt.x << prst->pt.y;

        double x = prst->pt.x + rect.x();
        double y = prst->pt.y + rect.y();

        CvPoint pt1, pt2;
        pt1.x = x - 40;
        pt1.y = y;
        pt2.x = x + 40;
        pt2.y = y;
        cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
        pt1.x = x;
        pt1.y = y - 40;
        pt2.x = x;
        pt2.y = y + 40;
        cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
    }
}


void MainWindow::setReadImage()
{
    ViewMainPage* pView = viewMainPage();
    if (!pView)
        return;

    DocumentView* v = currentView();
    if (!v)
        return;


    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), QDir::currentPath(),
            tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
    if (fileName.isEmpty()) {
        return;
    }

    //QApplication::setOverrideCursor(Qt::WaitCursor);
    QTimer::singleShot(1, [=] {
        QImage img;
        cv::Mat m = cv::imread(fileName.toLocal8Bit().toStdString().c_str(), cv::IMREAD_COLOR);
        mat_to_qimage(m, img);
        if (v) {
            v->mRoi->setWidth(img.width());
            v->mRoi->setHeight(img.height());

            v->document()->setImageInternal(img);
            v->imageView()->updateBuffer();

            v->updateLayout();
            pView->updateLayout();

        }
        //QApplication::restoreOverrideCursor();
    });

}

